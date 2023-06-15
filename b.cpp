#include<iostream>
using namespace std;




int main(int argc, char** argv[][]){
	cout << "number of arguments is " << argc<< endl;
	
	for(int i = 0; i< argc; i++){
	cout << "the argv is " << argv[i] << endl;
}
return 0;
}
