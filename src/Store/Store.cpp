#include "Store.h"


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
  std::string key;
  std::string value;
  bool term = false;
  
  for(std::string::size_type i = 0; i < input.size(); ++i) {
     
     if(type==2){
         // scanning a key
         if(value.size()==0){
            // not yet found start of key, might be string or otherwise
            if(input[i]==':' || input[i]==' '){
                continue; // still not found the start, keep looking
            } else if(input[i]=='\"'){
                // string value, set our scan to stop at a terminating '"'
                term=true;
            } else {
                // not a string, set our scan to stop at a terminating ','
                value+=input[i];  // this isn't a terminator, so add to value
            }
         } else {
             // we're adding chars. check for terminator
             if( (term && input[i]=='\"') || (!term && input[i]==',') || (!term && input[i]=='}') ){
                 // terminator found, add to internal map and reset
                 type=0;
                 size_t sz=value.size();
                 while(value[sz-1]==' ') --sz; // trim
                 value.resize(sz);
                 m_variables[key] = value;
                 key="";
                 value="";
                 term=false;
             } else {
                // just a char to add to value
                value+=input[i];
             }
         }
     }
     else if(input[i]=='\"') type++;
     else if(type==1)key+=input[i];
     else if(type==3)value+=input[i];
     else if(type==4){
       type=0;
       m_variables[key]=value;
       key="";
       value="";
       term=false;
     }
     
      
      /*
    if(input[i]!=',' &&  input[i]!='{' && input[i]!='}' && input[i]!='\"' && input[i]!=':' && input[i]!=',')pair<<input[i];
    else if(input[i]==':')pair<<" ";
    else if(input[i]==',') {
      std::cout<<" i = "<<i<<" pair = "<<pair<<std::endl;
    
      pair>>key>>value;
      */  
      //pair.clear();

      //}
  }
  
}

bool Store::Has(std::string key){

  return (m_variables.count(key)!=0);

}
