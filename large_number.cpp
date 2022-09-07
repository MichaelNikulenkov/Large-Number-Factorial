#include <string>
#include <algorithm>
#include <omp.h>
#include <fstream>
#include "large_number.h"

uint8_t LargeNumber::get_digit(size_t index) const {
	return _data[index];
}

uLL LargeNumber::additional_magnitude() const {
	return _additional_magnitude;
}

LargeNumber::LargeNumber(uLL value) {
	//разряды хранятся в обратном порядке
	do {
		uint8_t number = static_cast<uint8_t>(value % 10);
		_data.push_back(number);
		value /= 10;
	} while (value > 0);
}

size_t LargeNumber::digits_num() const {
	return _data.size();
}

std::string LargeNumber::to_str() const {
	std::vector<uint8_t> data_copy = _data;
	//посчитать степень десятки
	uLL magnitude = 0;
	if (_data.size() > 1) {
		auto it = std::find_if_not(_data.begin(), _data.end(), [](const uint8_t& v) { return v == 0; });
		magnitude = static_cast<uLL>(std::distance(_data.begin(), it)); //степень десятки / индекс первого ненулевого разряда
	}

	std::reverse(data_copy.begin() + magnitude, data_copy.end());
	std::for_each(data_copy.begin() + magnitude, data_copy.end(), [](uint8_t& v) { v += 48; });

	std::string result(data_copy.begin() + magnitude, data_copy.end());

	uLL total_magnitude = magnitude + _additional_magnitude;
	if (total_magnitude > 0)
		result += std::string(total_magnitude, '0');

	return result;
}

void LargeNumber::add_smaller_to_bigger(const std::vector<uint8_t>& smaller, std::vector<uint8_t>& bigger) {
	for (size_t j = 0; j < smaller.size(); ++j) {

		uint8_t val = smaller[j] + bigger[j];
		uint8_t residual = val % 10;
		uint8_t carry = (val - residual) / 10;

		bigger[j] = residual;

		if (carry != 0) {
			if (bigger.size() == 1) {
				bigger.push_back(carry);
			}
			else {
				if ((j + 1) > (bigger.size() - 1))
					bigger.push_back(carry);
				else
					bigger[j + 1] += carry;

				for (size_t k = j + 1; k < bigger.size(); ++k) {
					uint8_t residual = bigger[k] % 10;
					uint8_t carry = (bigger[k] - residual) / 10;
					if (carry == 0) {
						break;
					}
					else {
						bigger[k] = residual;
						if (k == bigger.size() - 1)
							bigger.push_back(carry);
						else
							bigger[k + 1] += carry;
					}
				}
			}
		}
	}
}

LargeNumber& LargeNumber::operator*=(const LargeNumber& number) {
	//умножение столбиком
	size_t my_size = this->digits_num();
	size_t other_size = number.digits_num();

	//посчитаем степени при десятках
	size_t my_magnitude = 0;
	if (_data.back() != 0) {
		for (size_t i = 0; i < my_size; i++)
			if (_data[i] == 0)
				++my_magnitude;
			else
				break;
	}

	size_t other_magnitude = 0;
	if (number.get_digit(other_size - 1) != 0) {
		for (size_t i = 0; i < other_size; i++)
			if (number.get_digit(i) == 0)
				++other_magnitude;
			else
				break;
	}

	//сумма результатов умножений каждого разряда меньшего числа на большее число
	std::vector<uint8_t> sum = { 0 };

	if ((my_size - my_magnitude) < (other_size - other_magnitude)) {
		//если разрядность после учета степеней при десятках у числа-аргумента больше

		//умножение большего числа на i-й разряд меньшего 
		for (size_t i = 0; i < my_size - my_magnitude; ++i) {
			//сдвиг на степень десятки
			size_t i_s = i + my_magnitude;

			std::vector<uint8_t> temp(i + 1, 0);

			for (size_t j = 0; j < other_size - other_magnitude; ++j) {
				//сдвиг на степень десятки
				size_t j_s = j + other_magnitude;

				uint8_t val = temp.back() + _data[i_s] * number.get_digit(j_s);
				uint8_t residual = val % 10;
				uint8_t carry = (val - residual) / 10;
				temp.back() = residual;
				temp.push_back(carry);
				if ((j_s == other_size - 1) && (carry == 0))
					temp.pop_back();
			}

			//добавление к общей сумме
			if (temp.size() > sum.size()) {
				add_smaller_to_bigger(sum, temp);
				//промежуточный результат сложения из temp поместить в sum
				sum.swap(temp);
			}
			else {
				add_smaller_to_bigger(temp, sum);
			}
		}
	}
	else {
		for (size_t i = 0; i < other_size - other_magnitude; ++i) {
			//сдвиг на степень десятки
			size_t i_s = i + other_magnitude;

			std::vector<uint8_t> temp(i + 1, 0);

			for (size_t j = 0; j < my_size - my_magnitude; ++j) {
				//сдвиг на степень десятки
				size_t j_s = j + my_magnitude;

				uint8_t val = temp.back() + _data[j_s] * number.get_digit(i_s);
				uint8_t residual = val % 10;
				uint8_t carry = (val - residual) / 10;
				temp.back() = residual;
				temp.push_back(carry);
				if ((j_s == my_size - 1) && (carry == 0))
					temp.pop_back();
			}

			//добавление к общей сумме
			if (temp.size() > sum.size()) {
				add_smaller_to_bigger(sum, temp);
				//промежуточный результат сложения из temp поместить в sum
				sum.swap(temp);
			}
			else {
				add_smaller_to_bigger(temp, sum);
			}
		}
	}

	//результат сложения из sum поместить в _data
	_data.swap(sum);
	_additional_magnitude += static_cast<uLL>(my_magnitude) + static_cast<uLL>(other_magnitude) + number.additional_magnitude();

	return *this;
}

void LargeNumber::parallel_mult_by(const LargeNumber& number) {
	//умножение столбиком
	int my_size = this->digits_num();
	int other_size = number.digits_num();

	//посчитаем степени при десятках
	int my_magnitude = 0;
	if (_data.back() != 0) {
		for (int i = 0; i < my_size; i++)
			if (_data[i] == 0)
				++my_magnitude;
			else
				break;
	}

	int other_magnitude = 0;
	if (number.get_digit(other_size - 1) != 0) {
		for (int i = 0; i < other_size; i++)
			if (number.get_digit(i) == 0)
				++other_magnitude;
			else
				break;
	}

	//сумма результатов умножений каждого разряда меньшего числа на большее число
	std::vector<std::vector<uint8_t>> thread_sums = {};

	if ((my_size - my_magnitude) < (other_size - other_magnitude)) {
		//если разрядность после учета степеней при десятках у числа-аргумента больше

		#pragma omp parallel default(shared)
		{
			int threads_num = omp_get_num_threads();
			int thread_id = omp_get_thread_num();

			#pragma omp single 
			{
				thread_sums.assign(threads_num, { 0 });
			}

			//умножение большего числа на i-й разряд меньшего 
			#pragma omp for schedule(dynamic, 1)
			for (int i = 0; i < my_size - my_magnitude; ++i) {
				//сдвиг на степень десятки
				int i_s = i + my_magnitude;

				std::vector<uint8_t> temp(i + 1, 0);

				for (int j = 0; j < other_size - other_magnitude; ++j) {
					//сдвиг на степень десятки
					int j_s = j + other_magnitude;

					uint8_t val = temp.back() + _data[i_s] * number.get_digit(j_s);
					uint8_t residual = val % 10;
					uint8_t carry = (val - residual) / 10;
					temp.back() = residual;
					temp.push_back(carry);
					if ((j_s == other_size - 1) && (carry == 0))
						temp.pop_back();
				}

				//добавление к общей сумме
				if (temp.size() > thread_sums[thread_id].size()) {
					add_smaller_to_bigger(thread_sums[thread_id], temp);
					//промежуточный результат сложения из temp поместить в sum
					thread_sums[thread_id].swap(temp);
				}
				else {
					add_smaller_to_bigger(temp, thread_sums[thread_id]);
				}
			}
		}
	}
	else {
		#pragma omp parallel default(shared)
		{
			int threads_num = omp_get_num_threads();
			int thread_id = omp_get_thread_num();

			#pragma omp single 
			{
				thread_sums.assign(threads_num, { 0 });
			}

			#pragma omp for schedule(dynamic, 1)
			for (int i = 0; i < other_size - other_magnitude; ++i) {
				//сдвиг на степень десятки
				int i_s = i + other_magnitude;

				std::vector<uint8_t> temp(i + 1, 0);

				for (int j = 0; j < my_size - my_magnitude; ++j) {
					//сдвиг на степень десятки
					int j_s = j + my_magnitude;

					uint8_t val = temp.back() + _data[j_s] * number.get_digit(i_s);
					uint8_t residual = val % 10;
					uint8_t carry = (val - residual) / 10;
					temp.back() = residual;
					temp.push_back(carry);
					if ((j_s == my_size - 1) && (carry == 0))
						temp.pop_back();
				}

				//добавление к общей сумме
				if (temp.size() > thread_sums[thread_id].size()) {
					add_smaller_to_bigger(thread_sums[thread_id], temp);
					//промежуточный результат сложения из temp поместить в sum
					thread_sums[thread_id].swap(temp);
				}
				else {
					add_smaller_to_bigger(temp, thread_sums[thread_id]);
				}
			}
		}
	}

	//локальные результаты в thread_sums[]
	//сложение методом редукции
	do {
		#pragma omp single 
		{
			if (thread_sums.size() % 2 != 0)
				thread_sums.push_back({ 0 });
		}

		#pragma omp for schedule(dynamic, 1)
		for (int i = 0; i < thread_sums.size(); i += 2) {
			if (thread_sums[i].size() < thread_sums[i + 1].size()) {
				add_smaller_to_bigger(thread_sums[i], thread_sums[i + 1]);
				thread_sums[i].swap(thread_sums[i + 1]);
			}
			else {
				add_smaller_to_bigger(thread_sums[i + 1], thread_sums[i]);
			}
		}

		#pragma omp single 
		{
			size_t size = thread_sums.size();
			for (int i = 0; i < size / 2; i++) {
				for (int j = i + 1; j < size - 2 - i; j += 2) {
					thread_sums[j].swap(thread_sums[j + 1]);
				}
				thread_sums.pop_back();
			}
		}
	} while (thread_sums.size() > 1);

	//результат сложения поместить в _data
	_data.swap(thread_sums[0]);
	_additional_magnitude += static_cast<uLL>(my_magnitude) + static_cast<uLL>(other_magnitude) + number.additional_magnitude();
}