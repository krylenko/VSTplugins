#include "FXEngine.h"
#include <cstdio>
#include <memory>
int main(){
  const double sr=48000; const char* names[]={"","MS20 LP","MS20 HP","Comb","Cheby"};
  struct C{FXType t; float p0,p1;} cs[]={{FX_CHEBY,0,0},{FX_CHEBY,.5f,0},{FX_CHEBY,.5f,.5f},{FX_CHEBY,.5f,1},{FX_MS20_LP,.7f,.9f},{FX_MS20_HP,.3f,.5f},{FX_COMB,.3f,.9f}};
  for(auto c:cs){ auto b=std::make_unique<FXBlock>(); b->sampleRate=sr; b->type=c.t; b->param0=b->eff_param0=c.p0; b->param1=b->eff_param1=c.p1; b->reset();
    double sum=0; int n=0; double silence=0;
    for(int i=0;i<(int)sr*3;i++){ float x=0.5f*std::sin(2*M_PI*220*i/sr); float y=b->process(x); if(i>=sr*2){sum+=y;n++;} }
    for(int i=0;i<(int)sr;i++){ float y=b->process(0.f); if(i>sr/2) silence+=y/(sr/2);} 
    printf("%-8s p0=%.2f p1=%.2f  DC with sine %+.5f   DC in silence %+.5f\n",names[c.t],c.p0,c.p1,sum/n,silence);}
}
