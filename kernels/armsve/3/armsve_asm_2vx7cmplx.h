/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2014, The University of Texas at Austin
   Copyright (C) 2020, The University of Tokyo

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
#define GEMM_2VX7CMPLX_MKER_LOOP_PLAIN_C(C0Re,C1Re,C2Re,C3Re,C4Re,C5Re,C6Re,C0Im,C1Im,C2Im,C3Im,C4Im,C5Im,C6Im,PT,AColRe,AColIm,B0Re,B1Re,B2Re,B3Re,B4Re,B5Re,B6Re,B0Im,B1Im,B2Im,B3Im,B4Im,B5Im,B6Im,BAddr,BRSBit) \
  GEMM_FMLA2_LD1R(C0Re,C0Im,PT,AColRe,AColIm,B0Re,BAddr,0) \
  GEMM_FMLA2_LD1R(C1Re,C1Im,PT,AColRe,AColIm,B1Re,BAddr,2) \
  GEMM_FMLA2_LD1R(C2Re,C2Im,PT,AColRe,AColIm,B2Re,BAddr,4) \
  GEMM_FMLA2_LD1R(C3Re,C3Im,PT,AColRe,AColIm,B3Re,BAddr,6) \
  GEMM_FMLA2_LD1R(C4Re,C4Im,PT,AColRe,AColIm,B4Re,BAddr,8) \
  GEMM_FMLA2_LD1R(C5Re,C5Im,PT,AColRe,AColIm,B5Re,BAddr,10) \
  GEMM_FMLA2_LD1R(C6Re,C6Im,PT,AColRe,AColIm,B6Re,BAddr,12) \
  GEMM_FMLX2_LD1R(C0Im,C0Re,PT,AColRe,AColIm,B0Im,BAddr,1) \
  GEMM_FMLX2_LD1R(C1Im,C1Re,PT,AColRe,AColIm,B1Im,BAddr,3) \
  GEMM_FMLX2_LD1R(C2Im,C2Re,PT,AColRe,AColIm,B2Im,BAddr,5) \
  GEMM_FMLX2_LD1R(C3Im,C3Re,PT,AColRe,AColIm,B3Im,BAddr,7) \
  GEMM_FMLX2_LD1R(C4Im,C4Re,PT,AColRe,AColIm,B4Im,BAddr,9) \
  GEMM_FMLX2_LD1R(C5Im,C5Re,PT,AColRe,AColIm,B5Im,BAddr,11) \
  GEMM_FMLX2_LD1R(C6Im,C6Re,PT,AColRe,AColIm,B6Im,BAddr,13) \
" add             "#BAddr", "#BRSBit", "#BAddr"   \n\t"

#define GEMM_2VX7CMPLX_MKER_LOOP_PLAIN_C_RESIDUAL(C0Re,C1Re,C2Re,C3Re,C4Re,C5Re,C6Re,C0Im,C1Im,C2Im,C3Im,C4Im,C5Im,C6Im,PT,AColRe,AColIm,B0Re,B1Re,B2Re,B3Re,B4Re,B5Re,B6Re,B0Im,B1Im,B2Im,B3Im,B4Im,B5Im,B6Im,BAddr,BRSBit) \
  GEMM_FMLA2(C0Re,C0Im,PT,AColRe,AColIm,B0Re) \
  GEMM_FMLA2(C1Re,C1Im,PT,AColRe,AColIm,B1Re) \
  GEMM_FMLA2(C2Re,C2Im,PT,AColRe,AColIm,B2Re) \
  GEMM_FMLA2(C3Re,C3Im,PT,AColRe,AColIm,B3Re) \
  GEMM_FMLA2(C4Re,C4Im,PT,AColRe,AColIm,B4Re) \
  GEMM_FMLA2(C5Re,C5Im,PT,AColRe,AColIm,B5Re) \
  GEMM_FMLA2(C6Re,C6Im,PT,AColRe,AColIm,B6Re) \
  GEMM_FMLX2(C0Im,C0Re,PT,AColRe,AColIm,B0Im) \
  GEMM_FMLX2(C1Im,C1Re,PT,AColRe,AColIm,B1Im) \
  GEMM_FMLX2(C2Im,C2Re,PT,AColRe,AColIm,B2Im) \
  GEMM_FMLX2(C3Im,C3Re,PT,AColRe,AColIm,B3Im) \
  GEMM_FMLX2(C4Im,C4Re,PT,AColRe,AColIm,B4Im) \
  GEMM_FMLX2(C5Im,C5Re,PT,AColRe,AColIm,B5Im) \
  GEMM_FMLX2(C6Im,C6Re,PT,AColRe,AColIm,B6Im)

#define CLEAR_COL14(Z00,Z01,Z02,Z03,Z04,Z05,Z06,Z07,Z08,Z09,Z10,Z11,Z12,Z13) \
  CLEAR_COL4(Z00,Z01,Z02,Z03) \
  CLEAR_COL4(Z04,Z05,Z06,Z07) \
  CLEAR_COL4(Z08,Z09,Z10,Z11) \
  CLEAR_COL2(Z12,Z13)

#define GEMM_FMULCMPLX_COL7(ZD0Re,ZD0Im,ZD1Re,ZD1Im,ZD2Re,ZD2Im,ZD3Re,ZD3Im,ZD4Re,ZD4Im,ZD5Re,ZD5Im,ZD6Re,ZD6Im,PT,Z0Re,Z0Im,Z1Re,Z1Im,Z2Re,Z2Im,Z3Re,Z3Im,Z4Re,Z4Im,Z5Re,Z5Im,Z6Re,Z6Im,ZFactorRe,ZFactorIm) \
  FMUL_COL2(ZD0Re,ZD0Im,Z0Re,Z0Im,ZFactorRe) \
  FMUL_COL2(ZD1Re,ZD1Im,Z1Re,Z1Im,ZFactorRe) \
  FMUL_COL2(ZD2Re,ZD2Im,Z2Re,Z2Im,ZFactorRe) \
  FMUL_COL2(ZD3Re,ZD3Im,Z3Re,Z3Im,ZFactorRe) \
  FMUL_COL2(ZD4Re,ZD4Im,Z4Re,Z4Im,ZFactorRe) \
  FMUL_COL2(ZD5Re,ZD5Im,Z5Re,Z5Im,ZFactorRe) \
  FMUL_COL2(ZD6Re,ZD6Im,Z6Re,Z6Im,ZFactorRe) \
  GEMM_FMLX2(ZD0Im,ZD0Re,PT,Z0Re,Z0Im,ZFactorIm) \
  GEMM_FMLX2(ZD1Im,ZD1Re,PT,Z1Re,Z1Im,ZFactorIm) \
  GEMM_FMLX2(ZD2Im,ZD2Re,PT,Z2Re,Z2Im,ZFactorIm) \
  GEMM_FMLX2(ZD3Im,ZD3Re,PT,Z3Re,Z3Im,ZFactorIm) \
  GEMM_FMLX2(ZD4Im,ZD4Re,PT,Z4Re,Z4Im,ZFactorIm) \
  GEMM_FMLX2(ZD5Im,ZD5Re,PT,Z5Re,Z5Im,ZFactorIm) \
  GEMM_FMLX2(ZD6Im,ZD6Re,PT,Z6Re,Z6Im,ZFactorIm)

#define GEMM_FMLACMPLX_COL7(ZD0Re,ZD0Im,ZD1Re,ZD1Im,ZD2Re,ZD2Im,ZD3Re,ZD3Im,ZD4Re,ZD4Im,ZD5Re,ZD5Im,ZD6Re,ZD6Im,PT,Z0Re,Z0Im,Z1Re,Z1Im,Z2Re,Z2Im,Z3Re,Z3Im,Z4Re,Z4Im,Z5Re,Z5Im,Z6Re,Z6Im,ZFactorRe,ZFactorIm) \
  GEMM_FMLACMPLX(ZD0Re,ZD0Im,PT,Z0Re,Z0Im,ZFactorRe,ZFactorIm) \
  GEMM_FMLACMPLX(ZD1Re,ZD1Im,PT,Z1Re,Z1Im,ZFactorRe,ZFactorIm) \
  GEMM_FMLACMPLX(ZD2Re,ZD2Im,PT,Z2Re,Z2Im,ZFactorRe,ZFactorIm) \
  GEMM_FMLACMPLX(ZD3Re,ZD3Im,PT,Z3Re,Z3Im,ZFactorRe,ZFactorIm) \
  GEMM_FMLACMPLX(ZD4Re,ZD4Im,PT,Z4Re,Z4Im,ZFactorRe,ZFactorIm) \
  GEMM_FMLACMPLX(ZD5Re,ZD5Im,PT,Z5Re,Z5Im,ZFactorRe,ZFactorIm) \
  GEMM_FMLACMPLX(ZD6Re,ZD6Im,PT,Z6Re,Z6Im,ZFactorRe,ZFactorIm)

#define GEMM_CCMPLX_LOAD_COL7_C(Z0Re,Z0Im,Z1Re,Z1Im,Z2Re,Z2Im,Z3Re,Z3Im,Z4Re,Z4Im,Z5Re,Z5Im,Z6Re,Z6Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_LOAD_FWD(Z0Re,Z0Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_LOAD_FWD(Z1Re,Z1Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_LOAD_FWD(Z2Re,Z2Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_LOAD_FWD(Z3Re,Z3Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_LOAD_FWD(Z4Re,Z4Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_LOAD_FWD(Z5Re,Z5Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_LOAD_FWD(Z6Re,Z6Im,PT,CAddr,CCS)

#define GEMM_CCMPLX_STORE_COL7_C(Z0Re,Z0Im,Z1Re,Z1Im,Z2Re,Z2Im,Z3Re,Z3Im,Z4Re,Z4Im,Z5Re,Z5Im,Z6Re,Z6Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_STORE_FWD(Z0Re,Z0Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_STORE_FWD(Z1Re,Z1Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_STORE_FWD(Z2Re,Z2Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_STORE_FWD(Z3Re,Z3Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_STORE_FWD(Z4Re,Z4Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_STORE_FWD(Z5Re,Z5Im,PT,CAddr,CCS) \
  GEMM_CCOLCMPLX_CONTIGUOUS_STORE_FWD(Z6Re,Z6Im,PT,CAddr,CCS)

