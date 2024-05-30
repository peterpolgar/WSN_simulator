#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "eparams.h"

int main(int argc, char const *argv[])
{
    if ( argc < 4 ) {
        std::cerr << "Give input file, and what parameters you want to write out. Exit.\n";
        exit(1);
    }
    
    bool is_uszkve = false;
    bool is_mure = false;
    bool is_yeah = false;
    bool is_watsup = false;
    bool is_neman = false;
    bool is_kvazi = false;
    
    for ( int i = 3; i < argc; ++i ) {
        if ( strcmp(argv[i], "uszkve") == 0 ) {
            is_uszkve = true;
        }else if ( strcmp(argv[i], "mure") == 0 ) {
            is_mure = true;
        }else if ( strcmp(argv[i], "yeah") == 0 ) {
            is_yeah = true;
        }else if ( strcmp(argv[i], "watsup") == 0 ) {
            is_watsup = true;
        }else if ( strcmp(argv[i], "neman") == 0 ) {
            is_neman = true;
        }else if ( strcmp(argv[i], "kvazi") == 0 ) {
            is_kvazi = true;
        }else {
            std::cerr << argv[i] << " is not a valid parameter name. Exit.\n";
            exit(2);
        }
    }
    
    enum class eTeszt {
        uszkve,
        mure,
        yeah,
        watsup,
        neman,
        kvazi
    };
    eTeszt tmp_enum;
    
    unsigned uszkve = 0;
    double mure = 0;
    short yeah[2]{ 0, 0 };
    double watsup[3]{ 0., 0., 0. };
    std::vector<int> neman;
    std::vector<double> kvazi;
    
    unsigned idobelyeg = 300, next_time_stamp = idobelyeg, period_time_msec = 25;
    unsigned long out_len = 1, tmp_len = 0;
    int time_interval_msec = std::atoi(argv[2]);
    
    std::ifstream is{argv[1], std::ios::in | std::ios::binary};
    if ( ! is ) {
        std::cerr << "Cannot open data file " << argv[1] << ". Exit.\n";
        exit(3);
    }
    
    char csv_name[55];
    strcpy(csv_name, argv[1]);
    for ( int i = 0; i < 55; ++i ) {
        if ( csv_name[i] == '.' ) {
            csv_name[i] = '\0';
            break;
        }
    }
    strcat(csv_name, ".csv");
    std::ofstream ki_file{ csv_name, std::ios::out };
    std::string ki_str{ "Time-stamp" }, tmp_str;
    if ( is_uszkve ) {
        ki_str += ";uszkve";
    }
    if ( is_mure ) {
        ki_str += ";mure";
    }
    if ( is_yeah ) {
        ki_str += ";yeah";
    }
    if ( is_watsup ) {
        ki_str += ";watsup";
    }
    if ( is_neman ) {
        ki_str += ";neman";
    }
    if ( is_kvazi ) {
        ki_str += ";kvazi";
    }
    ki_str += "\n";
    ki_file.write(ki_str.c_str(), ki_str.size());
    int count = 1;
    while ( out_len ) {
        std::cout << count << ". sor:\n";
        is.read((char*)&idobelyeg, sizeof(idobelyeg));
        while ( idobelyeg > next_time_stamp ) {
            if ( next_time_stamp % time_interval_msec == 0 ) {
                ki_file.write((std::to_string(next_time_stamp) + tmp_str).c_str(), ki_str.size());
            }
            next_time_stamp += period_time_msec;
        }
        next_time_stamp += period_time_msec;
        ki_str = std::to_string(idobelyeg);
        tmp_str = "";
        is.read((char*)&out_len, sizeof(out_len));
        std::cout << idobelyeg << ' ' << out_len << ' ' << std::flush;
        unsigned long ki_len = out_len;
        while ( ki_len ) {
            is.read((char*)&tmp_enum, sizeof(eTeszt::uszkve));
            ki_len -= sizeof(eTeszt::uszkve);
            if ( tmp_enum == eTeszt::uszkve ) {
                is.read((char*)&uszkve, sizeof(uszkve));
                ki_len -= sizeof(uszkve);
                std::cout << uszkve << ' '<< std::flush;
            }else if ( tmp_enum == eTeszt::mure ) {
                is.read((char*)&mure, sizeof(mure));
                ki_len -= sizeof(mure);
                std::cout << mure << ' ';
            }else if ( tmp_enum == eTeszt::yeah ) {
                is.read((char*)yeah, sizeof(yeah));
                ki_len -= sizeof(yeah);
                std::cout << yeah[0] << ' ' << yeah[1] << ' ';
            }else if ( tmp_enum == eTeszt::watsup ) {
                is.read((char*)watsup, sizeof(watsup));
                ki_len -= sizeof(watsup);
                std::cout << watsup[0] << ' ' << watsup[1] << ' ' << watsup[2] << ' ';
            }else if ( tmp_enum == eTeszt::neman ) {
                is.read((char*)&tmp_len, sizeof(tmp_len));
                neman.resize(tmp_len);
                is.read((char*)neman.data(), tmp_len * sizeof(int));
                ki_len -= sizeof(tmp_len) + tmp_len * sizeof(int);
                for ( unsigned j = 0; j < neman.size(); ++j ) {
                    std::cout << neman[j] << ' ';
                }
            }else if ( tmp_enum == eTeszt::kvazi ) {
                is.read((char*)&tmp_len, sizeof(tmp_len));
                kvazi.resize(tmp_len);
                is.read((char*)kvazi.data(), tmp_len * sizeof(double));
                ki_len -= sizeof(tmp_len) + tmp_len * sizeof(double);
                for ( unsigned j = 0; j < kvazi.size(); ++j ) {
                    std::cout << kvazi[j] << ' ';
                }
            }
        }
        if ( is_uszkve ) {
            tmp_str += ";" + std::to_string(uszkve);
        }
        if ( is_mure ) {
            tmp_str += ";" + std::to_string(mure);
        }
        if ( is_yeah ) {
            tmp_str += ";" + std::to_string(yeah[0]);
            tmp_str += "," + std::to_string(yeah[1]);
        }
        if ( is_watsup ) {
            tmp_str += ";" + std::to_string(watsup[0]);
            tmp_str += "," + std::to_string(watsup[1]);
            tmp_str += "," + std::to_string(watsup[2]);
        }
        if ( is_neman ) {
            tmp_str += ";" + std::to_string(neman[0]);
            for ( unsigned j = 1; j < neman.size(); ++j ) {
                tmp_str += "," + std::to_string(neman[j]);
            }
        }
        if ( is_kvazi ) {
            tmp_str += ";" + std::to_string(kvazi[0]);
            for ( unsigned j = 1; j < kvazi.size(); ++j ) {
                tmp_str += "," + std::to_string(kvazi[j]);
            }
        }
        tmp_str += "\n";
        ki_str = ki_str + tmp_str;
        if ( out_len && idobelyeg % time_interval_msec == 0 ) {
            ki_file.write(ki_str.c_str(), ki_str.size());
        }
        ++count;
        std::cout << '\n';
    }
    ki_file.close();
    is.close();
}