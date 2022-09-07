#include <iostream>
#include <omp.h>
#include <fstream>
#include "factorial.h"

int main() {
	uLL value = 0;
	std::cout << "Enter a number: " << std::endl;
	std::cin >> value;

	//вычисление факториала
	double time_start = omp_get_wtime();
	LargeNumber result = factorial(value);
	double time_end = omp_get_wtime();
	std::cout << "Time elapsed: " << time_end - time_start << " sec." << std::endl;

	std::cout << "Factorial: " << std::endl;
	std::string result_str = result.to_str();
	std::cout << result_str << std::endl;

	//сохранение результата
	std::ofstream out("result.txt");
	out << result_str;
	out.close();
	std::cout << "Saved to result.txt " << std::endl;

	std::cout << "Type anyting to exit..." << std::endl;
	int a = 0;
	std::cin >> a;
	return 0;
}