#pragma once
#include <iostream>
#include <vector>

typedef unsigned long long uLL;

//способ представления больших чисел
class LargeNumber {
public:
	LargeNumber(uLL value);

	//нумерация в обратном порядке
	uint8_t get_digit(size_t index) const;
	size_t digits_num() const;

	//общая разрядность числа - digits_num() + additional_magnitude()
	uLL additional_magnitude() const;

	std::string to_str() const;
	LargeNumber& operator*=(const LargeNumber& number);
	//параллельная реализация перемножения больших чисел
	void parallel_mult_by(const LargeNumber& number);

private:
	//добавить к числу с большей разрядностью число с меньшей
	//результат остается в числе с большей разрядностью
	static void add_smaller_to_bigger(const std::vector<uint8_t>& smaller, std::vector<uint8_t>& bigger);

	std::vector<uint8_t> _data = {};
	//число представлено как reverse(_data) * 10^(_additional_magnitude)
	uLL _additional_magnitude = 0;
};