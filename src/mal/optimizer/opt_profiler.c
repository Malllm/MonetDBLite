/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 1997 - July 2008 CWI, August 2008 - 2017 MonetDB B.V.
 */

/*
 * Collect properties for beautified variable rendering
 * All variables are tagged with the schema.table.column name if possible.
 */

#include "monetdb_config.h"
#include "mal_instruction.h"
#include "opt_prelude.h"
#include "opt_profiler.h"

/*
static struct{
    char *mod, *fcn;
    char *alias;
}mapping[]={
    {"algebra", "projectionpath", "projection"},
    {"algebra", "thetasubselect", "select"},
    {"algebra", "projection", "projection"},
    {"dataflow", "language", "parallel"},
    {"algebra", "subselect", "select"},
    {"sql", "projectdelta", "project"},
    {"algebra", "subjoin", "join"},
    {"language", "pass(nil)", "release"},
    {"mat", "packIncrement", "pack"},
    {"language", "pass", "release"},
    {"aggr", "subcount", "count"},
    {"sql", "subdelta", "project"},
    {"bat", "append", "append"},
    {"aggr", "subavg", "average"},
    {"aggr", "subsum", "sum"},
    {"aggr", "submin", "minimum"},
    {"aggr", "submax", "maximum"},
    {"aggr", "count", "count"},
    {"calc", "lng", "long"},
    {"sql", "bind", "bind"},
    {"batcalc", "hge", "hugeint"},
    {"batcalc", "dbl", "real"},
    {"batcalc", "flt", "real"},
    {"batcalc", "lng", "bigint"},
    {0,0,0}};
*/

int
OPTprofilerImplementation(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int i;
	InstrPtr p;
	char buf[BUFSIZ];
	str v;
	lng usec = GDKusec();

	(void) pci;
	(void) stk;
	(void) cntxt;

	for( i=0; i< mb->stop; i++){
		p= getInstrPtr(mb,i);
		if( p == NULL)
			continue;
		if ( getModuleId(p) == NULL || getFunctionId(p) == NULL)
			continue;
		if( getModuleId(p)== sqlRef && (getFunctionId(p)== bindRef || getFunctionId(p) == bindidxRef)){
			// we know the arguments are constant
			snprintf(getSTC(mb,getArg(p,0)),  2 * IDLENGTH, "%s.%s.%s", 
				getVarConstant(mb, getArg(p,p->retc +1)).val.sval,
				getVarConstant(mb, getArg(p,p->retc +2)).val.sval,
				getVarConstant(mb, getArg(p,p->retc +3)).val.sval);
		} else
		if( getModuleId(p)== sqlRef && getFunctionId(p)== tidRef){
			// we know the arguments are constant
			snprintf(getSTC(mb,getArg(p,0)), 2 * IDLENGTH, "%s.%s", 
				getVarConstant(mb, getArg(p,2)).val.sval,
				getVarConstant(mb, getArg(p,3)).val.sval);
		} else
		if( getModuleId(p)== batRef && (getFunctionId(p)== deltaRef || getFunctionId(p) == subdeltaRef)){
			// inherit property of first argument
			v = getSTC(mb,getArg(p,1));
			if(v != NULL)
				strncpy(getSTC(mb,getArg(p,0)),v, 2 * IDLENGTH);
		} else
		if( getModuleId(p)== sqlRef && getFunctionId(p)== projectdeltaRef){
			// inherit property of first argument
			v = getSTC(mb,getArg(p,1));
			if(v != NULL)
				strncpy(getSTC(mb,getArg(p,0)),v, 2 * IDLENGTH);
		} else
		if( getModuleId(p)== algebraRef && getFunctionId(p)== projectionRef){
			// inherit property of last argument
			v = getSTC(mb,getArg(p,p->argc-1));
			if( v != NULL)
				strncpy(getSTC(mb,getArg(p,0)),v, 2 * IDLENGTH);
		} else
		if( getModuleId(p)== algebraRef && (getFunctionId(p)== subselectRef || getFunctionId(p) == thetasubselectRef)){
			// inherit property of first argument
			v = getSTC(mb,getArg(p,p->retc));
			if( v != NULL)
				strncpy(getSTC(mb,getArg(p,0)),v, 2 * IDLENGTH);
		} else
		if( getModuleId(p)== algebraRef && (getFunctionId(p)== likesubselectRef || getFunctionId(p) == ilikesubselectRef)){
			// inherit property of first argument
			v = getSTC(mb,getArg(p,p->retc));
			if( v != NULL)
				strncpy(getSTC(mb,getArg(p,0)),v, 2 * IDLENGTH);
		} else
		if( getModuleId(p)== algebraRef && 
			( getFunctionId(p)== subjoinRef ||
			  getFunctionId(p) == subleftjoinRef ||
			  getFunctionId(p) == subthetajoinRef ||
			  getFunctionId(p) == subantijoinRef ||
			  getFunctionId(p) == subbandjoinRef ||
			  getFunctionId(p) == subrangejoinRef )){
			// inherit property of last argument
			v = getSTC(mb,getArg(p,p->retc) );
			if( v != NULL)
				strncpy(getSTC(mb,getArg(p,0)),v, 2 * IDLENGTH);
			v = getSTC(mb,getArg(p,p->retc + 1 ) );
			if( v != NULL)
				strncpy(getSTC(mb,getArg(p,1)),v, 2 * IDLENGTH);
		} 
	}
    /* Defense line against incorrect plans */
	/* Plan remains unaffected */
	//chkTypes(cntxt->fdout, cntxt->nspace, mb, FALSE);
	//chkFlow(cntxt->fdout, mb);
	//chkDeclarations(cntxt->fdout, mb);
	//
    /* keep all actions taken as a post block comment */
    snprintf(buf,256,"%-20s actions=%2d time=" LLFMT " usec","profiler",1,GDKusec() - usec);
    newComment(mb,buf);

	return 1;
}
