#pragma once
#include <caliper/Annotation.h>
void record_caliper_metadata();
template<typename T>
void setGlobal(const char* name, T val){
  cali::Annotation global_annot(name,CALI_ATTR_GLOBAL);
  global_annot.set(val);
}
