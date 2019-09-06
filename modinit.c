#include "orgel.h"

int main(int argc, char *argv[]){
  module *adsr = create_module("adsr", 0);
  
  adsr->input_ptr[2]._value.virtual_cv=4;
  adsr->input_ptr[3]._value.virtual_cv=0;
  adsr->input_ptr[4]._value.virtual_cv=60;
  adsr->input_ptr[5]._value.virtual_cv=0;
  adsr->input_ptr[6]._value.virtual_cv=0;
  adsr->input_ptr[7]._value.virtual_cv=60;

  for(int i=1; i<=adsr->input_ptr->len; ++i){
    printf("Input: %s, value=%d \n", adsr->input_ptr[i].name,
	   adsr->input_ptr[i]._value.virtual_cv);
  }

  adsr->input_ptr[1]._value.logic=0;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=0;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=1;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=1;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=1;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=1;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=1;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=1;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=1;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=0;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);
  adsr->input_ptr[1]._value.logic=0;
  adsr->tick(adsr, 1);
  printf ("Output: %d \n", adsr->output_ptr->_value.virtual_cv);



}
