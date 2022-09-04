/*   =========================================================
 *   DSP/BIOS LINK Configuration file.
 *
 *   CAUTION! This is a generated file.
 *            All changes will be lost.
 *
 *   This file was generated on NOV 12, 2021  12:48:49
 *   Target platform for DSP/BIOS LINK: OMAP3530
 *   =========================================================
 */

/*  ----------------------------------- DSP/BIOS LINK Headers       */
#include <dsplink.h>
#include <procdefs.h>
#include <_dsplink.h>

#if defined (__cplusplus)
EXTERN "C" {
#endif /* defined (__cplusplus) */

extern LINKCFG_Gpp LINKCFG_gppObject ;
extern LINKCFG_DspConfig OMAP3530_SHMEM_Config ;

/** ============================================================================
 *  @name   LINKCFG_config
 *
 *  @desc   Configuration object for the overall system.
 *  ============================================================================
 */
LINKCFG_Object LINKCFG_config = { &LINKCFG_gppObject,
                                        {&OMAP3530_SHMEM_Config, } ,
                                      }  ;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
