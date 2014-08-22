
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <string.h>

#include "main_osc.h"
#include "fpga_osc.h"
#include "math.h"
#include "complex.h"
#include "linAlg.h"

double dBfun(double x){
  x=fabs(x) ;
  if (x<1e-100) { x=1e-100 ; }
  return(20*log(x)/log(10.0)) ;
  }

double sqr(double x){
  return x*x ;
  }


int setSignal(double frq1, double frq2) ;
void setFrq(double frq1, double frq2) ;

/** Program name */
const char *g_argv0 = NULL;

/** Oscilloclope module parameters as defined in main module
 * @see rp_main_params
 */
float t_params[PARAMS_NUM] = { 0, 1e6, 0, 0, 0, 0, 0, 0, 0, 0 };






typedef struct options {
  int showXs ;
  int showRs ;
  int showLs ;
  int showCs ;
  int showRp ;
  int showLp ;
  int showCp ;
  int showQ ;
  int showGain ;
  int showZabs ;
  int showZphi ;
  double refR ;
  int lowDUT ;
  char *fileName ;
  int v1 ;
  } options_t ;


void printSIprefixed(double q, char *s){
  double q1=fabs(q) ;
 // printf("q1=%e ",q1) ;
  if( q1 >= 1e9   ) { fprintf(stderr,"%8.3f G%s",q/1e9  ,s) ; return ; }
  if( q1 >= 1e6   ) { fprintf(stderr,"%8.3f M%s",q/1e6  ,s) ; return ; }
  if( q1 >= 1e3   ) { fprintf(stderr,"%8.3f k%s",q/1e3  ,s) ; return ; }
  if( q1 >= 1e0   ) { fprintf(stderr,"%8.3f  %s",q/1e0  ,s) ; return ; }
  if( q1 >= 1e-3  ) { fprintf(stderr,"%8.3f m%s",q/1e-3 ,s) ; return ; }
  if( q1 >= 1e-6  ) { fprintf(stderr,"%8.3f u%s",q/1e-6 ,s) ; return ; }
  if( q1 >= 1e-9  ) { fprintf(stderr,"%8.3f n%s",q/1e-9 ,s) ; return ; }
  if( q1 >= 1e-12 ) { fprintf(stderr,"%8.3f p%s",q/1e-12,s) ; return ; }
                      fprintf(stderr,"%8.1e  %s",q/1e-12,s) ;
  }

void impedanceAnalyse(double fMeasure, complex uXX, complex uYY, options_t theOptions){
 complex uZ ;
  complex uShunt ;
  if( theOptions.lowDUT ){
    uShunt=uXX-uYY ;
    uZ=uYY ;
    } else {
    uShunt=uYY ;
    uZ=uXX-uYY ;
    }
  double  Rshunt=theOptions.refR ;
  complex iShunt=uShunt/Rshunt ;
  complex zz=uZ/iShunt ;

  complex yy=1/zz ;
  double Rs=__real__ zz  ;
  double Xs=__imag__ zz ;
  double Gp=__real__ yy  ;
  double Bp=__imag__ yy  ;
  double omega=2*M_PI*fMeasure ;
  double Ls=Xs / omega ;
  double Cs=-1.0/( Xs * omega ) ;
  double Rp=1/Gp ;
  double Lp=-1.0/(omega*Bp) ;
  double Cp=Bp/omega ;
  double zAbs=cabs(zz) ;
  double phiZ=atan2(Xs,Rs)*180/M_PI ;
  if ( phiZ < -180) { phiZ += 360.0 ; }
  if ( phiZ >  180) { phiZ -= 360.0 ; }
 
  double QQ=-1E10 ;
  if (theOptions.showLs) { QQ=Xs/Rs ; }
  if (theOptions.showCs) { QQ=-Xs/Rs ; }
  if (theOptions.showLp) { QQ=Xs/Rs ; }
  if (theOptions.showCp) { QQ=-Xs/Rs ; }

  if (theOptions.showLs) { fprintf(stderr,"Ls=") ; printSIprefixed(Ls,"H " ) ; }
  if (theOptions.showCs) { fprintf(stderr,"Cs=") ; printSIprefixed(Cs,"F " ) ; }
  if (theOptions.showRs) { fprintf(stderr,"Rs=") ; printSIprefixed(Rs,"Ohm " ) ; }
  if (theOptions.showXs) { fprintf(stderr,"Xs=") ; printSIprefixed(Xs,"Ohm " ) ; }
  if (theOptions.showLp) { fprintf(stderr,"Lp=") ; printSIprefixed(Lp,"H " ) ; }
  if (theOptions.showCp) { fprintf(stderr,"Cp=") ; printSIprefixed(Cp,"F " ) ; }
  if (theOptions.showRp) { fprintf(stderr,"Rp=") ; printSIprefixed(Rp,"Ohm " ) ; }
  if (theOptions.showQ)  { fprintf(stderr,"Q=%9.3f ",QQ) ; }
  if (theOptions.showZabs) { fprintf(stderr,"|z|=") ; printSIprefixed(zAbs,"Ohm " ) ; }
  if (theOptions.showZphi) { fprintf(stderr,"phi=%9.3f deg ",phiZ) ; }
  }


void analyseSignal(int size , float **s, double fSample, double fMeasure, FILE *outfp, options_t theOptions){
  int i ;
  double sigX[SIGNAL_LENGTH] ;
  double sigY[SIGNAL_LENGTH] ;
  for(i = 0; i < size ; i++) {
    sigX[i]=(int)s[1][i] ;
    sigY[i]=(int)s[2][i] ;
    }
  rmatrix matAX ;
  rmatrix matAY ;
  clearRmat(matAX,3) ;
  clearRmat(matAY,3) ;
  rvector rhsX ;
  rvector rhsY ;
  rvector f ;
  clearRvec(rhsX,3) ;
  clearRvec(rhsY,3) ;

  for(i = 0; i < size ; i++) {
    double t=i*2*M_PI*fMeasure/fSample ;
    f[0]=cos(t) ;
    f[1]=sin(t) ;
    f[2]=1 ;

    //sigX[i]=1000.0*co+55.0*si ;
    //sigY[i]=100.0*co+255.0*si ;
    for (int j=0 ; j<3 ; j++){ 
      for(int k=0 ; k<3 ; k++ ){
        matAX[j][k] += f[j]*f[k] ;
        matAY[j][k] += f[j]*f[k] ; }
      rhsX[j] += f[j]*sigX[i] ;
      rhsY[j] += f[j]*sigY[i] ; }
    }
  if( theOptions.v1 ){
    fprintf(stderr,"\nleast squares matrix:") ;
    rmdsp(matAX,3) ;
    fprintf(stderr,"least squares vectors X,Y:\n") ;
    for(int k=0 ; k<3 ; k++){
      fprintf(stderr,"%15.5e  %15.5e\n",rhsX[k],rhsY[k]) ;
      }
    }
  rsolv(matAX , rhsX , 3 ) ;
  rsolv(matAY , rhsY , 3 ) ;
  if( theOptions.v1 ){
    fprintf(stderr,"least squares solution vectors X,Y:\n") ;
    for(int k=0 ; k<3 ; k++){
      fprintf(stderr,"%15.5e  %15.5e\n",rhsX[k],rhsY[k]) ;
      }
    }
  double uX=rhsX[0] ;
  double vX=rhsX[1] ;
  double uY=rhsY[0] ;
  double vY=rhsY[1] ;
  double eEstiX=sqrt(sqr(uX)+sqr(vX)) ;
  double eEstiY=sqrt(sqr(uY)+sqr(vY)) ;
  double argX=atan2(uX,vX) ;
  double argY=atan2(uY,vY) ;
  double phiX=360.0/(2.0*M_PI)*argX ;
  double phiY=360.0/(2.0*M_PI)*argY ;
 
  
  double dPhi=phiX-phiY ;
  if ( dPhi < -180) { dPhi += 360.0 ; }
  if ( dPhi >  180) { dPhi -= 360.0 ; }
  if( eEstiX<0.001 ) { eEstiX=0.1 ; }
  if( eEstiY<0.001 ) { eEstiY=0.1 ; }
  double gain=eEstiY/eEstiX ;
  if ( outfp != NULL) {
    fprintf(outfp, "%12.1f , ",fMeasure);
    fprintf(outfp,"%10.5f , %8.4f , %12.4f ,\n",gain,dPhi,eEstiX) ;
    }
  complex uYY=uY-I*vY ;
  complex uXX=uX-I*vX ;
  impedanceAnalyse(fMeasure,uXX,uYY,theOptions) ;
  if (theOptions.showGain) { fprintf(stderr,"gain=%7.4f = %8.2f dB phi=%9.3f deg",gain,dBfun(gain),dPhi ) ; }
  if ( gain<0.005){ fprintf(stderr," LOWsig! ") ; }
  if( theOptions.v1 ){
    double sqSumX=0.0 ;
    double sqSumY=0.0 ; 
    double sqSumXres=0.0 ;
    double sqSumYres=0.0 ; 
    double maxX=0.0 ;
    double maxY=0.0 ;
    for(i = 0; i < size ; i++) {
      double t=i*2*M_PI*fMeasure/fSample ;
      double co=cos(t) ;
      double si=sin(t) ;
      if (abs(sigX[i]) >maxX ){ maxX=abs(sigX[i]) ;  }
      if (abs(sigY[i]) >maxY ){ maxY=abs(sigY[i]) ;  }
      sqSumX +=  sqr (sigX[i]) ;
      sqSumY +=  sqr (sigY[i]) ;
      sqSumXres +=  sqr (sigX[i]-uX*co-vX*si-rhsX[2]) ;
      sqSumYres +=  sqr (sigY[i]-uY*co-vY*si-rhsY[2]) ;
      }
    sqSumX=sqrt(sqSumX/size) ;
    sqSumY=sqrt(sqSumY/size) ;
    sqSumXres=sqrt(sqSumXres/size) ;
    sqSumYres=sqrt(sqSumYres/size) ;
    fprintf(stderr,"\n") ;
    fprintf(stderr,"A1=%8.2f phi1=%8.2f A2=%8.2f phi2=%8.2f\n",eEstiX,phiX,eEstiY,phiY) ;
    fprintf(stderr,"\n") ;
    fprintf(stderr,"maxX     =%15.3f maxY     =%15.3f\n",maxX,maxY) ;
    fprintf(stderr,"sqSumX   =%15.3f sqSumY   =%15.3f\n",sqSumX,sqSumY) ;
    fprintf(stderr,"residual energy after subtracting a*cos+b*sin+c\n") ;
    fprintf(stderr,"sqSumXres=%15.3f sqSumYres=%15.3f\n",sqSumXres,sqSumYres) ;
    }
  }



int doOneMeasurement( float **s , double fSample, double fMeasure, int isDummy, FILE *outfp, options_t theOptions){
  int retries = 150000;
  int sig_num, sig_len;
  int ret_val;
  int retVal=0 ;
  int error1=-1 ; 
  int size=16384 ;
  while(retries >= 0) {
    if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
      if (sig_len!=size){ fprintf(stderr,"Siglen error!\n") ; retVal=error1 ; break ; }
      if(isDummy) { return retVal ; }
      analyseSignal(size,s,fSample,fMeasure,outfp, theOptions) ;
      break;
      }
    if(retries-- == 0) {
      fprintf(stderr, "Signal scquisition was not triggered!\n");
      retVal=error1 ;
      break;
      }
    usleep(1000);
    }
  return retVal ;
  }

#define kHz (1e3)
#define MHz (1e6)


/** Print usage information */
void usage() {
  const char *format =
  "usage:\n"
  "  %s options\n"
  "  options are:\n"
  "  start NUMBER   set sweep start frequency \n"
  "  stop NUMBER    set sweep stop frequency \n"
  "  center NUMBER  set sweep center frequency \n"
  "  span NUMBER    set sweep span\n"
  "  n NUMBER       set number of frequencies in sweep, default is 10\n"
  "  ref NUMBER     set value of reference resistoor, default is 10Ohm\n"
  "  file NAME      select file NAME as output for gain/phase data\n"
  "  lowdut         select DUT is lowside connected\n"
  "  lin            select linear sweep\n"
  "  log            select logarithmic sweep, is default\n"
  "  the following options control the data that is output for impedance measurement:\n"
  "  Rs             output Rs of series equivalent impedance circuit Z=Rs+jXs\n"
  "  Xs             output Xs of series equivalent impedance circuit Z=Rs+jXs\n"
  "  Ls             output Ls of series equivalent impedance = resistor in series with inductor\n"
  "  Cs             output Cs of series equivalent impedance = resistor in series with capacitor\n"
  "  Rp             output Rp of parallel equivalent impedance\n"
  "  Lp             output Ls of parallel equivalent impedance = resistor in parallel with inductor\n"
  "  Q              output quality factor of impedance\n"
  "  Z              output absolute value of complex impedance value\n"
  "  phi            output phase angle in degrees of complex impedance value\n"
  "\n"
  "examples of usage:\n"
  "%s start 1e3 stop 200e3 log n 50 Rs Ls\n"
  "%s center 10.7e6 span 200e3 lin n 150 gain\n"
  "%s start 1e3 stop 50e6 n 200 file /tmp/out1.txt\n"
  "\n" ;
  fprintf(stderr,format, g_argv0, g_argv0, g_argv0, g_argv0);
  }



int main(int argc, char *argv[]){
  fprintf(stderr,"Gain/Phase/Impedance Analyser by M. Ossmann, Date 20.th Aug. 2014\n") ;
  g_argv0 = argv[0];
  if (argc<2){
    usage() ;
    exit(1) ;
    }
  options_t theOptions ;
  double f1=-1.0 ;
  double f2=-1.0 ;
  double fCenter=-1.0 ;
  double fSpan=-1.0 ;
  int nFrqs = 10 ;
  theOptions.refR=10 ;
  int linSweep=0 ;
  theOptions.showXs=0 ;
  theOptions.showRp=0 ;
  theOptions.showLp=0 ;
  theOptions.showCp=0 ;
  theOptions.showRs=0 ;
  theOptions.showLs=0 ;
  theOptions.showCs=0 ;
  theOptions.showQ=0 ;
  theOptions.showGain=0 ;
  theOptions.showZabs=0 ;
  theOptions.showZphi=0 ;
  theOptions.lowDUT=0 ;
  theOptions.v1=0 ;
  theOptions.fileName=NULL ;

  int k=1 ;
  while( k<argc ){
    //printf("scan k=%i arg=%s\n",k,argv[k]) ;
    if ( strcmp(argv[k], "file") == 0) { 
      if(k==argc-1){ fprintf(stderr,"file argument missing") ; return -1 ; } 
      theOptions.fileName = argv[k+1] ; k+=2 ;  continue ;
      }
    if ( strcmp(argv[k], "ref") == 0) { 
      if(k==argc-1){ fprintf(stderr,"ref argument missing") ; return -1 ; } 
      theOptions.refR = strtod(argv[k+1], NULL); k+=2 ;  continue ;
      }
    if ( strcmp(argv[k], "start") == 0) { 
      if(k==argc-1){ fprintf(stderr,"start argument missing") ; return -1 ; } 
      f1 = strtod(argv[k+1], NULL); k+=2 ;   continue ;
      }
    if ( strcmp(argv[k], "stop") == 0) { 
      if(k==argc-1){ fprintf(stderr,"stop argument missing") ; return -1 ; } 
      f2 = strtod(argv[k+1], NULL); k+=2 ;   continue ;
      }
    if ( strcmp(argv[k], "center") == 0) { 
      if(k==argc-1){ fprintf(stderr,"start argument missing") ; return -1 ; } 
      fCenter = strtod(argv[k+1], NULL); k+=2 ;  continue ;
      }
    if ( strcmp(argv[k], "span") == 0) { 
      if(k==argc-1){ fprintf(stderr,"stop argument missing") ; return -1 ; } 
      fSpan = strtod(argv[k+1], NULL); k+=2 ; 
      //printf("fSpan=%15.5e",fSpan) ;  
       continue ;
      }
    if ( strcmp(argv[k], "n") == 0) { 
      if(k==argc-1){ fprintf(stderr,"n argument missing") ; return -1 ; } 
      nFrqs = atoi(argv[k+1]); k+=2 ;   continue ;
      }
    if ( strcmp(argv[k], "log") == 0) { linSweep=0 ; k+=1 ;   continue ; }
    if ( strcmp(argv[k], "lin") == 0)  {  linSweep=1 ; k+=1 ;   continue ;  }
    if ( strcmp(argv[k], "Rs") == 0)   { theOptions.showRs=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "Xs") == 0)   { theOptions.showXs=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "Rp") == 0)   { theOptions.showRp=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "Lp") == 0)   { theOptions.showLp=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "Cp") == 0)   { theOptions.showCp=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "Ls") == 0)   { theOptions.showLs=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "Cs") == 0)   { theOptions.showCs=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "Q") == 0)    { theOptions.showQ=1  ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "gain") == 0) { theOptions.showGain=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "Z") == 0)    { theOptions.showZabs=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "phi") == 0)  { theOptions.showZphi=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "lowdut") == 0)  { theOptions.lowDUT=1 ; k+=1 ; continue ; }
    if ( strcmp(argv[k], "v1") == 0)   { theOptions.v1=1 ; k+=1 ; continue ; }
    fprintf(stderr,"illegal option: %s",argv[k]) ; 
    return -1 ;  
    }


//  fprintf(stderr,"fSpan=%15.5e\n",fSpan) ;  

  if ( nFrqs < 1 ) { fprintf(stderr,"nPoints must be >0 !\n") ; return -1 ; }
  double fStart=1e3 ;
  double fStop=50e6 ;
  if (( f1>0 ) && (f2>0)) { fStart=f1 ; fStop=f2 ; }
  if (( fSpan>=0 ) && (fCenter>0)) {
    fStart=fCenter-fSpan/2 ;
    fStop=fCenter+fSpan/2 ;
    }
  fprintf(stderr,"fStart=%15.5f kHz fStop=%15.5f kHz\n",fStart/kHz,fStop/kHz) ;
  double fMin=1*kHz ; 
  double fMax=50*MHz ;
  if ( (fStart < fMin) | (fStart > fMax) ) { fprintf(stderr,"fStart not allowed !\n") ; return -1 ; }
  if ( (fStop  < fMin) | (fStop  > fMax) ) { fprintf(stderr,"fStop not allowed !\n") ; return -1 ; }
  fprintf(stderr,"nPoints=%8d \n",nFrqs) ;
 
  FILE *outfp=NULL ;
//  char *fileName="/tmp/out1.txt" ;
  if ( theOptions.fileName != NULL ) {
    fprintf(stderr,"save data to file:%s\n",theOptions.fileName) ;
    outfp=fopen(theOptions.fileName,"w") ;
    if (outfp==NULL){ fprintf(stderr,"fopen error") ; exit(1) ; }
    fprintf(outfp,"; GPIanalyse data\n") ;
    }


  /* Acquisition size */
  uint32_t size = SIGNAL_LENGTH ;
  


  if (size != SIGNAL_LENGTH) { fprintf(stderr, "Invalid size: %i\n", size); return -1;  }
  if (size != 16384 ) { fprintf(stderr, "Invalid size: %i\n", size); return -1;  }
  
  /* Optional acquisition decimation */
  double fSample ;
  t_params[8] =0 ; fSample=125e6 ;
 // t_params[8] =1 ; fSample=125e6/8.0 ; // decimation 8
 // t_params[8] =2 ; fSample=125e6/64.0 ; // decimation 64
  /* Initialization of Oscilloscope application */
  if(rp_app_init() < 0) {
    fprintf(stderr, "rp_app_init() failed!\n");
    return -1;
    }
/* Setting of parameters in Oscilloscope main module */
  if(rp_set_params((float *)&t_params, PARAMS_NUM) < 0) {
    fprintf(stderr, "rp_set_params() failed!\n");
    return -1;
    }
  float **s;
  int i;
  s = (float **)malloc(SIGNALS_NUM * sizeof(float *));
  for(i = 0; i < SIGNALS_NUM; i++) {
    s[i] = (float *)malloc(SIGNAL_LENGTH * sizeof(float));
    }
/*
  linSweep=1 ;
  nFrqs=4 ;
  double fCenter=10.7e6 ;
  double fWidth=100e3 ;

  fStart=fCenter-fWidth/2 ;
  fStop=fCenter+fWidth/2 ;
//  fStart=1e3 ;  fStop=50e6 ; nFrqs=100 ;
//fStart=1e6 ;  fStop=fStart ; nFrqs=2 ;
*/
 
  double frq=1e3 ; 
  setSignal(frq,frq) ;

  int step ;
  double dFrq=0 ;
  if(nFrqs>1){
    if(linSweep){
      dFrq=(fStop-fStart)/(nFrqs-1.0) ;
      } else {
      dFrq=(log(fStop)-log(fStart))/(nFrqs-1.0) ;
      }  
    }
  for(step=0 ; step<nFrqs ; step++){
    if (linSweep){
      frq=fStart+dFrq*step ;
      } else {
      frq=exp(log(fStart)+dFrq*step) ;
      }
    if (frq<1e6){
      fprintf(stderr,"\n%5i f=%8.2f kHz ",step,frq/1e3);
      } else {
      fprintf(stderr,"\n%5i f=%8.4f MHz ",step,frq/1e6); }
    if (frq<30e3) {
    //  t_params[8] =1 ; fSample=125e6/8.0 ; // decimation 8
      t_params[8] =2 ; fSample=125e6/64.0 ; // decimation 64
      } else {
      t_params[8] =0 ; fSample=125e6 ;
      }
    if(rp_set_params((float *)&t_params, PARAMS_NUM) < 0) {
     fprintf(stderr, "rp_set_params() failed!\n");
     return -1;
     }
    //fprintf(stderr,"fSample=%10.3f kHz \n",fSample/1e3) ;
    setFrq(frq,frq) ;
    usleep(100000);
    //usleep(10000);
    doOneMeasurement(s,fSample,frq,1,outfp, theOptions) ;
    doOneMeasurement(s,fSample,frq,0,outfp, theOptions) ;
    }
//  fprintf(stderr,"\n\nend of stdOut\n");
  if ( outfp != NULL ){
    fclose(outfp) ;
    }

  if(rp_app_exit() < 0) {
    fprintf(stderr, "rp_app_exit() failed!\n");
    return -1;
    }
  fprintf(stderr, "\n") ;
  return 0;
  }




