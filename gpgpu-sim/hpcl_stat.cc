/*
 * hpcl_stat.cpp
 *
 *  Created on: Dec 28, 2015
 *      Author: mumichang
 */

#include "hpcl_stat.h"
#include <limits>
#include <iostream>

hpcl_stat::hpcl_stat() {
	// TODO Auto-generated constructor stub
  clear();
}

hpcl_stat::~hpcl_stat() {
	// TODO Auto-generated destructor stub
}

hpcl_stat::hpcl_stat(const std::string name) : _name(name) {

}

void hpcl_stat::clear() {

  _num_samples = 0;
  _sample_sum = 0;
  _min = 0;
  _max = 0;

  _min = std::numeric_limits<double>::quiet_NaN();
  _max = -std::numeric_limits<double>::quiet_NaN();

}

double hpcl_stat::avg() const
{
  //std::cout << "_sample_sum : " << _sample_sum << " _num_samples : " << _num_samples << std::endl;
  return _sample_sum / (double)_num_samples;
}

double hpcl_stat::max() const
{
  return _max;
}

double hpcl_stat::min() const
{
  return _min;
}

double hpcl_stat::sum() const
{
  return _sample_sum;
}

void hpcl_stat::add_sample(double val)
{
  ++_num_samples;
  _sample_sum += val;

  // NOTE: the negation ensures that NaN values are handled correctly!
  _max = !(val <= _max) ? val : _max;
  _min = !(val >= _min) ? val : _min;
}

//added by kh(041616)
double hpcl_stat::get_sample_no()
{
  return (double)_num_samples;
}
///
