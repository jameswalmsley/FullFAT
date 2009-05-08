#ifndef _FFTERM_ERROR_H_
#define _FFTERM_ERROR_H_

#define FFT_ERR_NONE					 0
#define FFT_ERR_GENERIC					-1
#define FFT_ERR_NULL_POINTER			-2
#define	FFT_ERR_NOT_ENOUGH_MEMORY		-3


#define FFT_ERR_CMD_NAME_TOO_LONG		-10
#define FFT_ERR_CMD_NOT_FOUND			-11
#define FFT_ERR_CMD_ALREADY_EXISTS		-12
#define FFT_ERR_CMD_PROMPT_TOO_LONG		-13


const FF_T_INT8 *FFTerm_GetErrMessage( FF_T_SINT32 iErrorCode);


#endif
