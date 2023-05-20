#include "../cabecalhos/records.hpp"

#include <sstream>
#include <iomanip>

// Converte uma variavel date em string
string date_to_str(date d){
    stringstream ss;
    ss << setfill('0') 
       << setw(4) << d.year << "-"
       << setw(2) << d.month << "-"
       << setw(2) << d.day << " "
       << setw(2) << d.hour << ":"
       << setw(2) << d.min << ":"
       << setw(2) << d.sec;
    return ss.str();
}


// Retorna um timestamp
date str_to_date(const string& s){
    smatch sm;
    regex timestamp_re("[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}");

    if(!regex_match(s, sm, timestamp_re)){
        throw invalid_argument("Invalid timestamp format");
    }

    date d;
    stringstream ss(s);
    char ignore;

    ss >> d.year >> ignore 
       >> d.month >> ignore 
       >> d.day >> ignore
       >> d.hour >> ignore
       >> d.min >> ignore
       >> d.sec;

    return d;
}

// Imprime os dados de uma entrada no stdout
void print_record(const record* regster){
    cout << "ID: " << regster->id << "\n"
         << "Title: " << regster->title << "\n"
         << "Year: " << regster->year << "\n"
         << "Autors: " << regster->autors << "\n"
         << "Mention: " << regster->mention << "\n"
         << "Timestamp: " << date_to_str(regster->timestamp) << "\n"
         << "Snippet: " << regster->snippet << "\n\n";
}
