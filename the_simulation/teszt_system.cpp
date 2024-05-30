#include <cstring>
#include <cstdlib>
#include <iostream>

int main()
{
    char out_file_name[55];
    strcpy(out_file_name, "te.st_2.out");
    char csv_gen_command[200];
    strcpy(csv_gen_command, "./csv_gen ");
    strcat(csv_gen_command, out_file_name);
    strcat(csv_gen_command, " 100 average_hopcount_of_received_frames average_sending_time_of_received_frames relative_delivery_error average_energy_of_nodes");
    std::cout << csv_gen_command << '\n';
    // system(csv_gen_command);
}