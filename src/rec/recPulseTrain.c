/* recPulseTrain.c */
/* base/src/rec  $Id$ */

/* recPulser.c - Record Support Routines for PulseTrain records */
/*
 * Author:      Janet Anderson
 * Date:     6/21/91
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  10-24-91        jba     New device support changes
 * .02  11-11-91        jba     Moved set and reset of alarm stat and sevr to macros
 * .03  02-05-92	jba	Changed function arguments from paddr to precord 
 * .04  02-28-92        jba     Changed get_precision,get_graphic_double,get_control_double
 * .05  02-28-92	jba	ANSI C changes
 * .06  04-10-92        jba     pact now used to test for asyn processing, not status
 * .07  04-18-92        jba     removed process from dev init_record parms
 * .08  06-02-92        jba     changed graphic/control limits for per,oper
 * .09  07-15-92        jba     changed VALID_ALARM to INVALID alarm
 * .10  07-16-92        jba     added invalid alarm fwd link test and chngd fwd lnk to macro
 * .11  09-10-92        jba     replaced get of hgv value with call to recGblGetLinkvalue
 * .12  10-10-92        jba     replaced code with recGblGetLinkValue call
 * .13  10-18-92        jba     pact now set in recGblGetLinkValue
 */ 

#include     <vxWorks.h>
#include     <types.h>
#include     <stdioLib.h>
#include     <lstLib.h>

#include        <alarm.h>
#include     <dbDefs.h>
#include     <dbAccess.h>
#include     <dbRecDes.h>
#include     <dbFldTypes.h>
#include     <devSup.h>
#include     <errMdef.h>
#include     <recSup.h>
#include     <pulseTrainRecord.h>

/* Create RSET - Record Support Entry Table*/
#define report NULL
#define initialize NULL
static long init_record();
static long process();
#define special NULL
static long get_value();
#define cvt_dbaddr NULL
#define get_array_info NULL
#define put_array_info NULL
#define get_units NULL
static long get_precision();
#define get_enum_str NULL
#define get_enum_strs NULL
#define put_enum_str NULL
static long get_graphic_double();
static long get_control_double();
#define get_alarm_double NULL

struct rset pulseTrainRSET={
     RSETNUMBER,
     report,
     initialize,
     init_record,
     process,
     special,
     get_value,
     cvt_dbaddr,
     get_array_info,
     put_array_info,
     get_units,
     get_precision,
     get_enum_str,
     get_enum_strs,
     put_enum_str,
     get_graphic_double,
     get_control_double,
     get_alarm_double };


struct ptdset { /* pulseTrain input dset */
     long          number;
     DEVSUPFUN     dev_report;
     DEVSUPFUN     init;
     DEVSUPFUN     init_record; /*returns: (-1,0)=>(failure,success)*/
     DEVSUPFUN     get_ioint_info;
     DEVSUPFUN     write_pt;/*(-1,0)=>(failure,success)*/
};

/* def for gtyp field */
#define SOFTWARE 1

/* defs for counter commands */
#define CTR_READ        0
#define CTR_CLEAR       1
#define CTR_START       2
#define CTR_STOP        3
#define CTR_SETUP       4

static void monitor();


static long init_record(ppt,pass)
    struct pulseTrainRecord	*ppt;
    int pass;
{
    struct ptdset *pdset;
    long status=0;

    if (pass==0) return(0);

    /* must have device support */
    if(!(pdset = (struct ptdset *)(ppt->dset))) {
         recGblRecordError(S_dev_noDSET,(void *)ppt,"pt: init_record");
         return(S_dev_noDSET);
    }
    /* get the hgv value if sgl is a constant*/
    if (ppt->sgl.type == CONSTANT )
    {
	 if(ppt->sgl.value.value!=0)
             ppt->hgv = ppt->sgl.value.value;
    }

    if (ppt->sgl.type == PV_LINK )
    {
        status = dbCaAddInlink(&(ppt->sgl), (void *) ppt, "SGV");
        if(status) return(status);
    } /* endif */

    /* must have write_pt functions defined */
    if( (pdset->number < 5) || (pdset->write_pt == NULL) ) {
         recGblRecordError(S_dev_missingSup,(void *)ppt,"pt: write_pt");
         return(S_dev_missingSup);
    }
    /* call device support init_record */
    if( pdset->init_record ) {
         if((status=(*pdset->init_record)(ppt))) return(status);
    }
    return(0);
}

static long process(ppt)
    struct pulseTrainRecord     *ppt;
{
    struct ptdset     *pdset = (struct ptdset *)(ppt->dset);
    long              status=0;
    long              options,nRequest;
    double            save;
    unsigned char    pact=ppt->pact;


    /* must have  write_pt functions defined */
    if( (pdset==NULL) || (pdset->write_pt==NULL) ) {
         ppt->pact=TRUE;
         recGblRecordError(S_dev_missingSup,(void *)ppt,"write_pt");
         return(S_dev_missingSup);
    }

    /* get soft hgv value when sgl is a DB_LINK and gtyp from Software */
    if (!ppt->pact && ppt->gtyp == SOFTWARE){
         options=0;
         nRequest=1;
         status=recGblGetLinkValue(&(ppt->sgl),(void *)ppt,DBR_SHORT,
              &(ppt->sgv),&options,&nRequest);

         if(status==0){
              if(ppt->sgv != ppt->osgv){ /* hgv changed */
                   if(ppt->sgv==0){
                        save=ppt->dcy;
                        ppt->dcy=0.0;
			status=(*pdset->write_pt)(ppt);
                        ppt->dcy=save;
                   	if(status!=0) {
                            recGblSetSevr(ppt,WRITE_ALARM,INVALID_ALARM);
                        }
                   }
                   ppt->osgv=ppt->sgv;
              }
         }
     }

     if (status==0 && (ppt->gtyp!=SOFTWARE || ppt->sgv!=0))
	status=(*pdset->write_pt)(ppt);

     /* check if device support set pact */
     if ( !pact && ppt->pact ) return(0);
     ppt->pact = TRUE;

     if(status==-1)status = 0;

     ppt->udf=FALSE;
     recGblGetTimeStamp(ppt);

     /* check event list */
     monitor(ppt);

     /* process the forward scan link record */
     recGblFwdLink(ppt);

     ppt->pact=FALSE;
     return(status);
}

static long get_value(ppt,pvdes)
    struct pulseTrainRecord             *ppt;
    struct valueDes     *pvdes;
{
    pvdes->field_type = DBF_SHORT;
    pvdes->no_elements=1;
    (short *)pvdes->pvalue = &ppt->val;
    return(0);
}
static long get_precision(paddr,precision)
    struct dbAddr *paddr;
    long          *precision;
{
    struct pulseTrainRecord    *ppt=(struct pulseTrainRecord *)paddr->precord;

    *precision = ppt->prec;
    if(paddr->pfield == (void *)&ppt->val) return(0);
    recGblGetPrec(paddr,precision);

    return(0);
}

static long get_graphic_double(paddr,pgd)
    struct dbAddr *paddr;
    struct dbr_grDouble *pgd;
{
    struct pulseTrainRecord     *ppt=(struct pulseTrainRecord *)paddr->precord;

    if(paddr->pfield==(void *)&ppt->val
    || paddr->pfield==(void *)&ppt->per
    || paddr->pfield==(void *)&ppt->oper){
        pgd->upper_disp_limit = ppt->hopr;
        pgd->lower_disp_limit = ppt->lopr;
    } else recGblGetGraphicDouble(paddr,pgd);

    return(0);
}

static long get_control_double(paddr,pcd)
    struct dbAddr *paddr;
    struct dbr_ctrlDouble *pcd;
{
    struct pulseTrainRecord     *ppt=(struct pulseTrainRecord *)paddr->precord;

    if(paddr->pfield==(void *)&ppt->val
    || paddr->pfield==(void *)&ppt->per){
        pcd->upper_ctrl_limit = ppt->hopr;
        pcd->lower_ctrl_limit = ppt->lopr;
    } else recGblGetControlDouble(paddr,pcd);

    return(0);
}

static void monitor(ppt)
    struct pulseTrainRecord             *ppt;
{
    unsigned short  monitor_mask;

    monitor_mask = recGblResetAlarms(ppt);
    monitor_mask |= (DBE_VALUE | DBE_LOG);
    db_post_events(ppt,&ppt->val,monitor_mask);
    if(ppt->oper != ppt->per){
         db_post_events(ppt,&ppt->per,monitor_mask);
         ppt->oper=ppt->per;
    }
    if(ppt->odcy != ppt->dcy){
         db_post_events(ppt,&ppt->dcy,monitor_mask);
         ppt->odcy=ppt->dcy;
    }
    return;
}
