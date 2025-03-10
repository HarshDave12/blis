/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2014, The University of Texas at Austin
   Copyright (C) 2018 - 2019, Advanced Micro Devices, Inc.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name(s) of the copyright holder(s) nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "blis.h"

void bli_trmm_ru_ker_var2
     (
       const obj_t*     a,
       const obj_t*     b,
       const obj_t*     c,
       const cntx_t*    cntx,
       const cntl_t*    cntl,
             thrinfo_t* thread_par
     )
{
	const num_t     dt        = bli_obj_exec_dt( c );
	const dim_t     dt_size   = bli_dt_size( dt );

	      doff_t    diagoffb  = bli_obj_diag_offset( b );

	const pack_t    schema_a  = bli_obj_pack_schema( a );
	const pack_t    schema_b  = bli_obj_pack_schema( b );

	      dim_t     m         = bli_obj_length( c );
	      dim_t     n         = bli_obj_width( c );
	      dim_t     k         = bli_obj_width( a );

	const void*     buf_a     = bli_obj_buffer_at_off( a );
	const inc_t     cs_a      = bli_obj_col_stride( a );
	const dim_t     pd_a      = bli_obj_panel_dim( a );
	const inc_t     ps_a      = bli_obj_panel_stride( a );

	const void*     buf_b     = bli_obj_buffer_at_off( b );
	const inc_t     rs_b      = bli_obj_row_stride( b );
	const dim_t     pd_b      = bli_obj_panel_dim( b );
	const inc_t     ps_b      = bli_obj_panel_stride( b );

	      void*     buf_c     = bli_obj_buffer_at_off( c );
	const inc_t     rs_c      = bli_obj_row_stride( c );
	const inc_t     cs_c      = bli_obj_col_stride( c );

	// Detach and multiply the scalars attached to A and B.
	obj_t scalar_a, scalar_b;
	bli_obj_scalar_detach( a, &scalar_a );
	bli_obj_scalar_detach( b, &scalar_b );
	bli_mulsc( &scalar_a, &scalar_b );

	// Grab the addresses of the internal scalar buffers for the scalar
	// merged above and the scalar attached to C.
	const void* buf_alpha = bli_obj_internal_scalar_buffer( &scalar_b );
	const void* buf_beta  = bli_obj_internal_scalar_buffer( c );

	// Alias some constants to simpler names.
	const dim_t     MR         = pd_a;
	const dim_t     NR         = pd_b;
	const dim_t     PACKMR     = cs_a;
	const dim_t     PACKNR     = rs_b;

	// Query the context for the micro-kernel address and cast it to its
	// function pointer type.
	gemm_ukr_vft gemm_ukr = bli_cntx_get_l3_vir_ukr_dt( dt, BLIS_GEMM_UKR, cntx );

	const void* one        = bli_obj_buffer_for_const( dt, &BLIS_ONE );
	const char* a_cast     = buf_a;
	const char* b_cast     = buf_b;
	      char* c_cast     = buf_c;
	const char* alpha_cast = buf_alpha;
	const char* beta_cast  = buf_beta;

	/*
	   Assumptions/assertions:
	     rs_a == 1
	     cs_a == PACKMR
	     pd_a == MR
	     ps_a == stride to next micro-panel of A
	     rs_b == PACKNR
	     cs_b == 1
	     pd_b == NR
	     ps_b == stride to next micro-panel of B
	     rs_c == (no assumptions)
	     cs_c == (no assumptions)
	*/

	// Safety trap: Certain indexing within this macro-kernel does not
	// work as intended if both MR and NR are odd.
	if ( ( bli_is_odd( PACKMR ) && bli_is_odd( NR ) ) ||
	     ( bli_is_odd( PACKNR ) && bli_is_odd( MR ) ) ) bli_abort();

	// If any dimension is zero, return immediately.
	if ( bli_zero_dim3( m, n, k ) ) return;

	// Safeguard: If the current panel of B is entirely below its diagonal,
	// it is implicitly zero. So we do nothing.
	if ( bli_is_strictly_below_diag_n( diagoffb, k, n ) ) return;

	// If there is a zero region to the left of where the diagonal of B
	// intersects the top edge of the panel, adjust the pointer to C and
	// treat this case as if the diagonal offset were zero. This skips over
	// the region that was not packed. (Note we assume the diagonal offset
	// is a multiple of MR; this assumption will hold as long as the cache
	// blocksizes are each a multiple of MR and NR.)
	if ( diagoffb > 0 )
	{
		n        -= diagoffb;
		c_cast   += diagoffb * cs_c * dt_size;
		diagoffb  = 0;
	}

	// If there is a zero region below where the diagonal of B intersects the
	// right side of the block, shrink it to prevent "no-op" iterations from
	// executing.
	if ( -diagoffb + n < k )
	{
		k = -diagoffb + n;
	}

	// Compute number of primary and leftover components of the m and n
	// dimensions.
	const dim_t n_iter = n / NR + ( n % NR ? 1 : 0 );
    const dim_t n_left = n % NR;

    const dim_t m_iter = m / MR + ( m % MR ? 1 : 0 );
    const dim_t m_left = m % MR;

	// Determine some increments used to step through A, B, and C.
	const inc_t rstep_a = ps_a * dt_size;

	const inc_t cstep_b = ps_b * dt_size;

	const inc_t rstep_c = rs_c * MR * dt_size;
	const inc_t cstep_c = cs_c * NR * dt_size;

	auxinfo_t aux;

	// Save the pack schemas of A and B to the auxinfo_t object.
	bli_auxinfo_set_schema_a( schema_a, &aux );
	bli_auxinfo_set_schema_b( schema_b, &aux );

	// The 'thread' argument points to the thrinfo_t node for the 2nd (jr)
	// loop around the microkernel. Here we query the thrinfo_t node for the
	// 1st (ir) loop around the microkernel.
	thrinfo_t* thread = bli_thrinfo_sub_node( thread_par );
	thrinfo_t* caucus = bli_thrinfo_sub_node( thread );

	// Query the number of threads and thread ids for each loop.
	//const dim_t jr_nt  = bli_thrinfo_n_way( thread );
	//const dim_t jr_tid = bli_thrinfo_work_id( thread );
	const dim_t ir_nt  = bli_thrinfo_n_way( caucus );
	const dim_t ir_tid = bli_thrinfo_work_id( caucus );

	dim_t jr_start, jr_end, jr_inc;
	dim_t ir_start, ir_end, ir_inc;

	// Note that we partition the 2nd loop into two regions: the triangular
	// part of C, and the rectangular portion.
	dim_t n_iter_tri;
	dim_t n_iter_rct;

	if ( bli_is_strictly_above_diag_n( diagoffb, k, n ) )
	{
		// If the entire panel of B does not intersect the diagonal, there is
		// no triangular region, and therefore we can skip the first set of
		// loops.
		n_iter_tri = 0;
		n_iter_rct = n_iter;
	}
	else
	{
		// If the panel of B does intersect the diagonal, compute the number of
		// iterations in the triangular (or trapezoidal) region by dividing NR
		// into the number of rows in B. (There should never be any remainder
		// in this division.) The number of iterations in the rectangular region
		// is computed as the remaining number of iterations in the n dimension.
		n_iter_tri = ( k + diagoffb ) / NR + ( ( k + diagoffb ) % NR ? 1 : 0 );
		n_iter_rct = n_iter - n_iter_tri;
	}

	// Use round-robin assignment of micropanels to threads in the 2nd and
	// 1st loops for the initial triangular region of B (if it exists).
	// NOTE: We don't need to call bli_thread_range_rr() here since we
	// employ a hack that calls for each thread to execute every iteration
	// of the jr and ir loops but skip all but the pointer increment for
	// iterations that are not assigned to it.

	const char* b1 = b_cast;
	      char* c1 = c_cast;

	// Loop over the n dimension (NR columns at a time).
	for ( dim_t j = 0; j < n_iter_tri; ++j )
	{
		const doff_t diagoffb_j = diagoffb - ( doff_t )j*NR;

		// Determine the offset to and length of the panel that was packed
		// so we can index into the corresponding location in A.
		const dim_t off_b0111 = 0;
		const dim_t k_b0111   = bli_min( k, -diagoffb_j + NR );

		const char* a1  = a_cast;
		      char* c11 = c1;

		const dim_t n_cur = ( bli_is_not_edge_f( j, n_iter, n_left )
		                      ? NR : n_left );

		// Initialize our next panel of B to be the current panel of B.
		const char* b2 = b1;

		// If the current panel of B intersects the diagonal, scale C
		// by beta. If it is strictly below the diagonal, scale by one.
		// This allows the current macro-kernel to work for both trmm
		// and trmm3.
		{
			// Compute the panel stride for the current diagonal-
			// intersecting micro-panel.
			inc_t ps_b_cur  = k_b0111 * PACKNR;
			      ps_b_cur += ( bli_is_odd( ps_b_cur ) ? 1 : 0 );
			      ps_b_cur *= dt_size;

			if ( bli_trmm_my_iter_rr( j, thread ) ) {

			// Loop over the m dimension (MR rows at a time).
			for ( dim_t i = 0; i < m_iter; ++i )
			{
				if ( bli_trmm_my_iter_rr( i, caucus ) ) {

				const dim_t m_cur = ( bli_is_not_edge_f( i, m_iter, m_left )
				                      ? MR : m_left );

				const char* a1_i = a1 + off_b0111 * PACKMR * dt_size;

				// Compute the addresses of the next panels of A and B.
				const char* a2 = a1;
				if ( bli_is_last_iter_rr( i, m_iter, 0, 1 ) )
				{
					a2 = a_cast;
					b2 = b1;
				}

				// Save addresses of next panels of A and B to the auxinfo_t
				// object.
				bli_auxinfo_set_next_a( a2, &aux );
				bli_auxinfo_set_next_b( b2, &aux );

				// Invoke the gemm micro-kernel.
				gemm_ukr
				(
				  m_cur,
				  n_cur,
				  k_b0111,
				  ( void* )alpha_cast,
				  ( void* )a1_i,
				  ( void* )b1,
				  ( void* )beta_cast,
				  c11, rs_c, cs_c,
				  &aux,
				  ( cntx_t* )cntx
				);
				}

				a1  += rstep_a;
				c11 += rstep_c;
			}
			}

			b1 += ps_b_cur;
		}

		c1 += cstep_c;
	}

	// If there is no rectangular region, then we're done.
	if ( n_iter_rct == 0 ) return;

	// Determine the thread range and increment for the 2nd and 1st loops for
	// the remaining rectangular region of B.
	// NOTE: The definition of bli_thread_range_slrr() will depend on whether
	// slab or round-robin partitioning was requested at configure-time.
	// NOTE: Parallelism in the 1st loop is disabled for now.
	bli_thread_range_slrr( thread, n_iter_rct, 1, FALSE, &jr_start, &jr_end, &jr_inc );
	bli_thread_range_slrr( caucus, m_iter,     1, FALSE, &ir_start, &ir_end, &ir_inc );

	// Advance the start and end iteration offsets for the rectangular region
	// by the number of iterations used for the triangular region.
	      jr_start += n_iter_tri;
	      jr_end   += n_iter_tri;
	dim_t jb0       = n_iter_tri;

	// Save the resulting value of b1 from the previous loop since it represents
	// the starting point for the rectangular region.
	b_cast = b1;

	// Loop over the n dimension (NR columns at a time).
	for ( dim_t j = jr_start; j < jr_end; j += jr_inc )
	{
		// NOTE: We must index through b_cast differently since it contains
		// the starting address of the rectangular region (which is already
		// n_iter_tri logical iterations through B).
		b1 = b_cast + (j-jb0) * cstep_b;
		c1 = c_cast +  j      * cstep_c;

		const dim_t n_cur = ( bli_is_not_edge_f( j, n_iter, n_left )
		                      ? NR : n_left );

		// Initialize our next panel of B to be the current panel of B.
		const char* b2 = b1;

		// If the current panel of B intersects the diagonal, scale C
		// by beta. If it is strictly below the diagonal, scale by one.
		// This allows the current macro-kernel to work for both trmm
		// and trmm3.
		{
			// Loop over the m dimension (MR rows at a time).
			for ( dim_t i = ir_start; i < ir_end; i += ir_inc )
			{
				const char* a1  = a_cast + i * rstep_a;
				      char* c11 = c1     + i * rstep_c;

				const dim_t m_cur = ( bli_is_not_edge_f( i, m_iter, m_left )
				                      ? MR : m_left );

				// Compute the addresses of the next panels of A and B.
				const char* a2 = bli_trmm_get_next_a_upanel( a1, rstep_a, ir_inc );
				if ( bli_is_last_iter_slrr( i, m_iter, ir_tid, ir_nt ) )
				{
					a2 = a_cast;
					b2 = bli_trmm_get_next_b_upanel( b1, cstep_b, jr_inc );
				}

				// Save addresses of next panels of A and B to the auxinfo_t
				// object.
				bli_auxinfo_set_next_a( a2, &aux );
				bli_auxinfo_set_next_b( b2, &aux );

				// Invoke the gemm micro-kernel.
				gemm_ukr
				(
				  m_cur,
				  n_cur,
				  k,
				  ( void* )alpha_cast,
				  ( void* )a1,
				  ( void* )b1,
				  ( void* )one,
				  c11, rs_c, cs_c,
				  &aux,
				  ( cntx_t* )cntx
				);
			}
		}
	}
}

//PASTEMAC(ch,fprintm)( stdout, "trmm_ru_ker_var2: a1", MR, k_b0111, a1, 1, MR, "%4.1f", "" );
//PASTEMAC(ch,fprintm)( stdout, "trmm_ru_ker_var2: b1", k_b0111, NR, b1_i, NR, 1, "%4.1f", "" );

