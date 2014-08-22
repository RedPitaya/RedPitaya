#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <string.h>


#include "math.h"
#include "complex.h"
#include "linAlg.h"


void rout(double x){
  printf(" %9.5f ",x);
  }

void rmdsp(rmatrix a , int  n){
  int i,j,n1;
  n1=n ;
  printf("\n") ;
  for( i=0 ; i<n1 ; i++ ){
    for ( j=0 ; j<n1 ; j++){
      rout(a[i][j]) ; 
      }
    printf("\n") ;
    }
  }

void rvdsp(rvector b , int  n){
  int i,n1;
  n1=n ;
  printf("\n") ;
  for( i=0 ; i<n1 ; i++ ){
    rout(b[i]) ; 
    printf("\n") ;
    }
  }

void clearRmat(rmatrix a , int  n){
  int i,j,n1;
  n1=n ;
  for( i=0 ; i<n1 ; i++ ){
    for ( j=0 ; j<n1 ; j++){
      a[i][j]=0 ; 
      }
    }
  }

void clearRvec(rvector a , int  n){
  int i,n1;
  n1=n ;
  for( i=0 ; i<n1 ; i++ ){
    a[i]=0 ; }
  }

void rsolv(rmatrix a , rvector b , int  n ) {
  int i,j,k,maxpos,n1 ; 
  double tmp,y ;
  double max ;
  n1=n ;
  for ( j=0 ; j<n1 ; j++){
    max=0 ;
    maxpos=j ;
    for ( k=j ; k<n1 ; k++ ){
      if ( abs(a[k][j]) > max ){ max=abs(a[k][j]) ; maxpos=k ; }
      }
    for ( k=j ; k<n1 ; k++ ){
      tmp=a[j][k] ; a[j][k]=a[maxpos][k] ; a[maxpos][k]=tmp ;
      }
    tmp=b[j] ; b[j]=b[maxpos] ; b[maxpos]=tmp ;
    y=1/a[j][j] ;
    for ( k=0 ; k<n1  ; k++ ){ a[j][k] = a[j][k] * y ; }
    b[j]=b[j]*y ;
    for ( i=0 ; i<n1  ; i++ ){ 
      if ( i != j ){
        y=a[i][j] ;
        for ( k=0 ; k<n1  ; k++ ){ a[i][k] -= y*a[j][k] ; }
	b[i] -= y*b[j] ;
        }
      }
    }  
  }    

