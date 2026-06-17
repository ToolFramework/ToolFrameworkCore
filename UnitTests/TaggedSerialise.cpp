#include <iostream>
#include <TaggedSerialisableObject.h>
#include <SerialisableObject.h>

using namespace ToolFramework;

struct Bob : public TaggedSerialisableObject{

  int a;
  std::vector<int> vec;
 
  std::string GetType(){ return "bob";}
  unsigned int GetVersion(){ return 1;}
  bool Print(){
    std::cout<<a<<std::endl;
    return true;
  }
  bool Serialise(BinaryStream &bs){

    bs & a;
    bs & vec;
    
    return true;
  }
  
};

struct Bill : public TaggedSerialisableObject{

  std::vector<int> vec;
 
  std::string GetType(){ return "bill";}
  unsigned int GetVersion(){ return 1;}
  bool Print(){ return true;}
  bool Serialise(BinaryStream &bs){

    bs & vec;
    
    return true;
  }
  
};

int main(){

  Bob bob;
  Bill bill;
  
  BinaryStream bs;
  bs.Bopen("test", NEW, UNCOMPRESSED);

  bob.a=23;
  bs & bob;
  
  bill.vec.resize(1000);
  bs & bill;
  
  bob.a= 46;
  bs & bob;

  bs & bill;
  bs.Bclose();

  std::cout<<"read"<<std::endl;

   BinaryStream bs2;
   bs2.Bopen("test", READ, UNCOMPRESSED);

   Tag tag;
   Bob bob2;
    
   bs2.m_write = false;

   while (bs2 & tag){
     tag.Print();
     if(tag.type !="bob"){
       tag.Skip(bs2);
       continue;
     }
     bs2 & bob2;
     bob2.Print();
     
   }

   bs2.Bclose();

  
  return 0;
}
