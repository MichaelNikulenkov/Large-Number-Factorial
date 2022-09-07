#pragma once
#include <iostream>
#include <vector>

typedef unsigned long long uLL;

//������ ������������� ������� �����
class LargeNumber {
public:
	LargeNumber(uLL value);

	//��������� � �������� �������
	uint8_t get_digit(size_t index) const;
	size_t digits_num() const;

	//����� ����������� ����� - digits_num() + additional_magnitude()
	uLL additional_magnitude() const;

	std::string to_str() const;
	LargeNumber& operator*=(const LargeNumber& number);
	//������������ ���������� ������������ ������� �����
	void parallel_mult_by(const LargeNumber& number);

private:
	//�������� � ����� � ������� ������������ ����� � �������
	//��������� �������� � ����� � ������� ������������
	static void add_smaller_to_bigger(const std::vector<uint8_t>& smaller, std::vector<uint8_t>& bigger);

	std::vector<uint8_t> _data = {};
	//����� ������������ ��� reverse(_data) * 10^(_additional_magnitude)
	uLL _additional_magnitude = 0;
};