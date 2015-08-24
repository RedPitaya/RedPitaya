
#define MAXDIM 10
typedef double rvector[MAXDIM]  ;
typedef double rmatrix[MAXDIM][MAXDIM]  ;

void rout(double x) ;
void rmdsp(rmatrix a , int  n) ;
void rvdsp(rvector b , int  n) ;
void clearRmat(rmatrix a , int  n) ;
void clearRvec(rvector a , int  n) ;
void rsolv(rmatrix a , rvector b , int  n ) ;
