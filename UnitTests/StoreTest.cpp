#include <iostream>
#include <Store.h>

using namespace ToolFramework;

int test_counter=0;

template <typename T> int Test(T &a, T &b, std::string message=""){
test_counter++;

if(a!=b){
    std::cout<<"ERROR "<<test_counter<<" "<<message<<": "<<a<<"!="<<b<<std::endl;
    return test_counter;
}
return 0;

}


int main(){

int ret=0;
bool pass=true;

int a=1;
short b=2;
long c=3;
float d=4.4;
double e=5.5;
bool f=true;
char g='h';
std::string h="hello world";

unsigned int i=6;
const int j=7;


Store store;

store.Set("a",a);
store.Set("b",b);
store.Set("c",c);
store.Set("d",d);
store.Set("e",e);
store.Set("f",f);
store.Set("g",g);
store.Set("h",h);
store.Set("i",i);
store.Set("j",j);
store.Set("k","hello world");
store.Set("l",2);
store.Set("m",4.4);
store.Set("n",true);

int a2=0;
short b2=0;
long c2=0;
float d2=0;
double e2=0;
bool f2=false;
char g2='0';
std::string h2="";
unsigned int i2=0;
std::string k2="";
short l2=0;
float m2=0;
bool n2=false;


 pass= pass && (store.Get("a",a2));
 pass= pass && (store.Get("b",b2));
 pass= pass && (store.Get("c",c2));
 pass= pass && (store.Get("d",d2));
 pass= pass && (store.Get("e",e2));
 pass= pass && (store.Get("f",f2));
 pass= pass && (store.Get("g",g2));
 pass= pass && (store.Get("h",h2));
 pass= pass && (store.Get("i",i2));
 const int j2=store.Get<int>("j");
 pass= pass && (store.Get("k",k2));
 pass= pass && (store.Get("l",l2));
 pass= pass && (store.Get("m",m2));
 pass= pass && (store.Get("n",n2));


bool tmp=true;
ret+=Test(pass,tmp, "Get Fail");

ret+=Test(a,a2);
ret+=Test(b,b2);
ret+=Test(c,c2);
ret+=Test(d,d2);
ret+=Test(e,e2);
ret+=Test(f,f2);
ret+=Test(g,g2);
ret+=Test(h,h2);
ret+=Test(i,i2);
ret+=Test(j,j2);
ret+=Test(h,k2);
ret+=Test(b,l2);
ret+=Test(d,m2);
ret+=Test(f,n2);

store.Print();


return ret;

}
