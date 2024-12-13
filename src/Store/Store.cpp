#include "Store.h"

namespace ToolFramework {

Store::Store(){}


bool Store::Initialise(std::istream& inputstream, std::string filename){
    
    bool all_ok = true;
    
    std::string line;
    while (getline(inputstream,line)){
      if (line.size()>0){
        if (line.at(0)=='#') continue;
        std::string key="";
        std::string value="";
        std::stringstream stream(line);
        if(stream>>key>>value){
            
            // continue reading any further tokens on this line, appending to value as a space separated list.
            // we also pre- and post-pend with '"' characters, so a line of 'myval    cat  dog	potato #comment'
            // will produce a value of '"cat dog potato"'
            value='"'+value;
            std::string tmp;
            stream>>tmp;
            while(tmp.length() && tmp[0]!='#'){
              value+=" "+tmp;
              tmp="";
              stream>>tmp;
            }
            
            value+='"';
            
            if(value!="\"\"") m_variables[key]=value;
        } else {
          std::clog<<"\033[38;5;196m WARNING!!!: Store::Initialise failed to parse line '"<<line;
          if(!filename.empty()) std::clog<<"' in file '"<<filename;
          std::clog<<"' \033[0m"<<std::endl;
          all_ok = false;
        }
      }
    }
    
    return all_ok;
    
}

bool Store::Initialise(std::string filename){
  
  bool all_ok;
  
  std::ifstream file(filename.c_str());
  
  if(file.is_open()){
    
    all_ok = Initialise(file);
    file.close();
    
  } else{
    
    std::clog<<"\033[38;5;196m WARNING!!!: Config file "<<filename<<" does not exist no config loaded \033[0m"<<std::endl;
    return false;
    
  }
  
  return all_ok;
}

void Store::Print(){
  
  for (std::map<std::string,std::string>::iterator it=m_variables.begin(); it!=m_variables.end(); ++it){
    
    std::clog<< it->first << " => " << it->second <<std::endl;
    
  }
  
}


void Store::Delete(){

  m_variables.clear();


}


void Store::JsonParser(std::string input){ 

  int type=0;
  std::string key="";
  std::string value="";
  int bracket_counter=0;

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

    if ((input[i] == '\n' || input[i] == '\r') && type != 4) continue;

    if(input[i]=='\"' && type<5){
      if(type==4) value+='"';
      type++;
      if(type==4) value+='"';
    }
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
     else if(type==4  && input[i]=='{'){ value.replace(value.length()-1, 1, "{"); type=6; }
     else if(type==6  && input[i]=='{'){ value+=input[i]; bracket_counter++; }
     else if(type==6  && input[i]=='}'){ value+=input[i]; bracket_counter--;if(bracket_counter==-1){ type=5; bracket_counter=0;} }
     else if(type==3  && input[i]=='['){ value+=input[i]; type=7; }
     else if(type==7  && input[i]=='['){ value+=input[i]; bracket_counter++; }
     else if(type==7  && input[i]==']'){ value+=input[i]; bracket_counter--;if(bracket_counter==-1){ type=5; bracket_counter=0;} }
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
    out=StringStrip(m_variables[name]);
    return true;
  }
  return false;
}

bool Store::Get(std::string name, bool &out){
  if(m_variables.count(name)>0){
    std::string tmp=StringStrip(m_variables[name]);
    if(tmp=="true") out=true;
    else if(tmp=="false") out=false;
    else if(tmp=="" || tmp=="0") out=false;
    else out=true;
    return true;
    
  }
  return false;

}

bool Store::Get(std::string name, Store &out){
  if(m_variables.count(name)>0 && StringStrip(m_variables[name])[0]=='{'){
    out.JsonParser(StringStrip(m_variables[name]));
      return true;
 }
 return false;
  
}

void Store::Set(std::string name, std::string in){
  std::stringstream stream;
  stream<<"\""<<in<<"\"";
  m_variables[name]=stream.str();
}

void Store::Set(std::string name, const char* in){
  std::stringstream stream;
  stream<<"\""<<in<<"\"";
  m_variables[name]=stream.str();
}

void Store::Set(std::string name,std::vector<std::string> in){
  std::stringstream stream;
  std::string tmp="[";
  for(unsigned int i=0; i<in.size(); i++){
    stream<<"\""<<in.at(i)<<"\"";
    tmp+=stream.str();
    if(i!=in.size()-1)tmp+=',';
    stream.str("");
    stream.clear();
  }
  tmp+=']';
  m_variables[name]=tmp;
  
}

std::string Store::StringStrip(std::string in){

  if(in.length() && in[0]=='"' && in[in.length()-1]=='"') return in.substr(1,in.length()-2);
  return in;

}

bool Store::Destring(std::string key){

  if(!m_variables.count(key)) return false;
  m_variables[key]=StringStrip(m_variables[key]);
  return true;

}

std::ostream& operator<<(std::ostream& stream, const Store& s){
  stream<<"{";
  bool first=true;
  for(auto it=s.m_variables.begin(); it!=s.m_variables.end(); ++it){
      if (!first) stream<<", ";
      stream<<"\""<<it->first<<"\":"<< it->second;
      first=false;
  }
  stream<<"}";
  return stream;
}
  
}
