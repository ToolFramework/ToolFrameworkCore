#include "Store.h"

using namespace ToolFramework;

Store::Store(){}


bool Store::Initialise(std::string filename){
  
  std::ifstream file(filename.c_str());
  std::string line;
  
  if(file.is_open()){
    
    while (getline(file,line)){
      if (line.size()>0){
	if (line.at(0)=='#')continue;
	std::string key;
	std::string value;
	std::stringstream stream(line);
	if(stream>>key>>value) m_variables[key]=value;
      }
      
    }
    file.close();
  }
  else{
    std::cout<<"\033[38;5;196m WARNING!!!: Config file "<<filename<<" does not exist no config loaded \033[0m"<<std::endl;
    return false;
  }
  
  return true;
}

void Store::Print(){
  
  for (std::map<std::string,std::string>::iterator it=m_variables.begin(); it!=m_variables.end(); ++it){
    
    std::cout<< it->first << " => " << it->second <<std::endl;
    
  }
  
}


void Store::Delete(){

  m_variables.clear();


}


void Store::JsonParser(std::string input){ 

  int type=0;
  std::string key="";
  std::string value="";

  for(std::string::size_type i = 0; i < input.size(); ++i) {
   
    //std::cout<<"i="<<i<<" , "<<input[i]<<" , type="<<type<<std::endl;
 
     //type 0011112233444444550011112233333333001111223366666665500111122337777777555
     //     { "key" : "value" , "key" : value , "key" : {......} , "key" : [......] }

     // types: 0 - pre key
     //        1 - key
     //        2 - postkey
     //        3 - value
     //        4 - string value
     //        5 - post value
     //        6 - object
     //        7 - array

     if(input[i]=='\"' && type<5) type++;
     else if(type==1) key+=input[i];
     else if(input[i]==':' && type==2) type=3;   
     else if((input[i]==',' || input[i]=='}') && (type==5 || type==3)){
       type=0;
       //std::cout<<"key="<<key<<" , value="<<value<<std::endl;
       m_variables[key]=value;
       key="";
       value="";
     }
     else if(type==3  && !(input[i]==' ' || input[i]=='{' || input[i]=='[')) value+=input[i];
     else if(type==3  && input[i]=='{'){ value+=input[i]; type=6; }
     else if(type==6  && input[i]=='}'){ value+=input[i]; type=5; }
     else if(type==3  && input[i]=='['){ value+=input[i]; type=7; }
     else if(type==7  && input[i]==']'){ value+=input[i]; type=5; }
     else if(type==4 || type==6 || type==7) value+=input[i];
  }

}
  


bool Store::Has(std::string key){

  return (m_variables.count(key)!=0);

}


std::vector<std::string> Store::Keys(){

  std::vector<std::string> ret;

  for(std::map<std::string, std::string>::iterator it= m_variables.begin(); it!= m_variables.end(); it++){

    ret.push_back(it->first);

  }

  return ret;

}


bool Store::Get(std::string name, std::string &out){
  
  if(m_variables.count(name)>0){ 
    out=m_variables[name];
    return true;
  }
  return false;
}

