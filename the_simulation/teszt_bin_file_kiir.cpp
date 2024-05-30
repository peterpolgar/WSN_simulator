#include <vector>
#include <fstream>
#include <cstring>

int main()
{
    enum class eTeszt {
        uszkve,
        mure,
        yeah,
        watsup,
        neman,
        kvazi
    };
    eTeszt tmp_enum;
    
    unsigned idobelyeg = 300;
    unsigned uszkve = 1;
    double mure = 2.;
    short yeah[2] { 3, 4 };
    double watsup[3]{ 5., 6., 7. };
    std::vector<int> neman{ 666, 777, 888 };
    std::vector<double> kvazi{ 8.1, 9.2, 10.3, 11.4 };
    
    std::ofstream os{"yeah.out", std::ios::out | std::ios::binary};
    char *out_str = new char[2000000];
    // elso sor kiirasa
    unsigned long out_len = 0, tmp_len = 0;
    memcpy(out_str, reinterpret_cast<char*>(&idobelyeg), sizeof(idobelyeg));
    out_len = sizeof(eTeszt::uszkve) * 6 + sizeof(uszkve) + sizeof(mure) + sizeof(yeah) + sizeof(watsup) + neman.size() * sizeof(int) + sizeof(tmp_len) + kvazi.size() * sizeof(double) + sizeof(tmp_len);
    memcpy(out_str + sizeof(idobelyeg), reinterpret_cast<char*>(&out_len), sizeof(out_len));
    out_len = sizeof(idobelyeg) + sizeof(out_len);
    tmp_enum = eTeszt::uszkve;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_enum), sizeof(eTeszt::uszkve));
    out_len += sizeof(eTeszt::uszkve);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&uszkve), sizeof(uszkve));
    out_len += sizeof(uszkve);
    tmp_enum = eTeszt::mure;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_enum), sizeof(eTeszt::mure));
    out_len += sizeof(eTeszt::mure);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&mure), sizeof(mure));
    out_len += sizeof(mure);
    tmp_enum = eTeszt::yeah;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_enum), sizeof(eTeszt::yeah));
    out_len += sizeof(eTeszt::yeah);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&yeah), sizeof(yeah));
    out_len += sizeof(yeah);
    tmp_enum = eTeszt::watsup;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_enum), sizeof(eTeszt::watsup));
    out_len += sizeof(eTeszt::watsup);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&watsup), sizeof(watsup));
    out_len += sizeof(watsup);
    tmp_enum = eTeszt::neman;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_enum), sizeof(eTeszt::neman));
    out_len += sizeof(eTeszt::neman);
    tmp_len = neman.size();
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_len), sizeof(tmp_len));
    out_len += sizeof(tmp_len);
    tmp_len *= sizeof(int);
    memcpy(out_str + out_len, reinterpret_cast<char*>(neman.data()), tmp_len);
    out_len += tmp_len;
    tmp_enum = eTeszt::kvazi;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_enum), sizeof(eTeszt::kvazi));
    out_len += sizeof(eTeszt::kvazi);
    tmp_len = kvazi.size();
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_len), sizeof(tmp_len));
    out_len += sizeof(tmp_len);
    tmp_len *= sizeof(double);
    memcpy(out_str + out_len, reinterpret_cast<char*>(kvazi.data()), tmp_len);
    out_len += tmp_len;
    os.write(out_str, out_len);
    
    // masodik sor kiirasa
    std::vector<eTeszt> changed_variables;
    idobelyeg = 400;
    uszkve = 22;
    changed_variables.push_back(eTeszt::uszkve);
    yeah[1] = 8;
    changed_variables.push_back(eTeszt::yeah);
    neman[2] = 999;
    changed_variables.push_back(eTeszt::neman);
    memcpy(out_str, reinterpret_cast<char*>(&idobelyeg), sizeof(idobelyeg));
    out_len = 0;
    for ( unsigned i = 0; i < changed_variables.size(); ++i ) {
        if ( changed_variables[i] == eTeszt::uszkve ) {
            out_len += sizeof(uszkve) + sizeof(eTeszt::uszkve);
        }else if ( changed_variables[i] == eTeszt::yeah ) {
            out_len += sizeof(yeah) + sizeof(eTeszt::yeah);
        }else if ( changed_variables[i] == eTeszt::neman ) {
            out_len += neman.size() * sizeof(int) + sizeof(eTeszt::neman) + sizeof(tmp_len);
        }
    }
    memcpy(out_str + sizeof(idobelyeg), reinterpret_cast<char*>(&out_len), sizeof(out_len));
    out_len = sizeof(idobelyeg) + sizeof(out_len);
    for ( unsigned i = 0; i < changed_variables.size(); ++i ) {
        if ( changed_variables[i] == eTeszt::uszkve ) {
            tmp_enum = eTeszt::uszkve;
            memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_enum), sizeof(eTeszt::uszkve));
            out_len += sizeof(eTeszt::uszkve);
            memcpy(out_str + out_len, reinterpret_cast<char*>(&uszkve), sizeof(uszkve));
            out_len += sizeof(uszkve);
        }else if ( changed_variables[i] == eTeszt::yeah ) {
            tmp_enum = eTeszt::yeah;
            memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_enum), sizeof(eTeszt::yeah));
            out_len += sizeof(eTeszt::yeah);
            memcpy(out_str + out_len, reinterpret_cast<char*>(&yeah), sizeof(yeah));
            out_len += sizeof(yeah);
        }else if ( changed_variables[i] == eTeszt::neman ) {
            tmp_enum = eTeszt::neman;
            memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_enum), sizeof(eTeszt::neman));
            out_len += sizeof(eTeszt::neman);
            tmp_len = neman.size();
            memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_len), sizeof(tmp_len));
            out_len += sizeof(tmp_len);
            tmp_len *= sizeof(int);
            memcpy(out_str + out_len, reinterpret_cast<char*>(neman.data()), tmp_len);
            out_len += tmp_len;
        }
    }
    os.write(out_str, out_len);
    
    // closing row
    idobelyeg = 500;
    memcpy(out_str, reinterpret_cast<char*>(&idobelyeg), sizeof(idobelyeg));
    out_len = 0;
    memcpy(out_str + sizeof(idobelyeg), reinterpret_cast<char*>(&out_len), sizeof(out_len));
    out_len = sizeof(idobelyeg) + sizeof(out_len);
    os.write(out_str, out_len);
    
    os.close();
}