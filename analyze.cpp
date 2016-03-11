#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;

double calculate_percentage(double value, double total)
{
   return (value/total)*100;
}

int main(int argc, char* argv[])
{
   if(argc != 2)
   {
      cerr << "Usage " << argv[0] << "output_n_verts.txt" << endl;
      exit(1);
   }
   string filename = argv[1];
   ifstream ifile;
   ifile.open(filename.c_str());
   string line;
   double total = 0;
   double three = 0;
   double four = 0;
   double five = 0;
   double six = 0;
   while(getline(ifile, line))
   {
      vector<string> split_words;
      boost::split(split_words, line, boost::is_any_of(" "));
      for(auto it = split_words.begin(); it != split_words.end();)
      {
         total++;
         it++;
         if(*it == "K3")
            three++;
         else if(*it == "K4")
            four++;
         else if(*it == "K5")
            five++;
         else if(*it == "K6")
            six++;
         break;
      }
   }
   cout << "K3 " << "Number: " << three << " Percentage: " << calculate_percentage(three, total) << endl;
   cout << "K4 " << "Number: " << four << " Percentage: " << calculate_percentage(four, total) << endl;
   cout << "K5 " << "Number: " << five << " Percentage: " << calculate_percentage(five, total) << endl;
   cout << "K6 " << "Number: " << six << " Percentage: " << calculate_percentage(six, total) << endl;
   return 0;
}
