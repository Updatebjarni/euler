#include"orgel.h"
#include<stdlib.h>

int e_min(int a, int b){
  return (a < b) ? a : b;
  }

int e_max(int a, int b){
  return (a > b) ? a : b;
  }

int e_dist(int a, int b){
  return abs(a-b);
  }
