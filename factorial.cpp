#include <omp.h>
#include "large_number.h"
#include "factorial.h"

LargeNumber factorial(uLL value) {
	if (value > 1) {
		//результаты промежуточных перемножений
		std::vector<LargeNumber> mid_results = {};

		#pragma omp parallel shared(mid_results, value)
		{
			int threads_num = omp_get_num_threads();
			int thread_id = omp_get_thread_num();
			#pragma omp single 
			{
				std::cout << "Using " << threads_num << " threads. Change OpenMP environment variables to adjust" << std::endl;
				mid_results.assign(threads_num, LargeNumber(1));
				std::cout << "Calculating partial products..." << std::endl;
			}

			#pragma omp for schedule(dynamic, 1)
			for (int i = 2; i <= value; ++i) {
				mid_results[thread_id] *= LargeNumber(i);
			}
		}

		//для максимизации загрузки процессора на последнем этапе перемножения происходят
		//в последновательном порядке с использованием параллельной версии операции умножения
		//лучшая альтернатива по сравнению с редукцией
		std::cout << "Calculating final product..." << std::endl;
		for (int i = 1; i < mid_results.size(); ++i) {
			mid_results[0].parallel_mult_by(mid_results[i]);
		}

		return mid_results[0];
	}
	else {
		return LargeNumber(1);
	}
}