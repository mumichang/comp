/*
 * hpcl_stat.h
 *
 *  Created on: Dec 28, 2015
 *      Author: mumichang
 */

#ifndef GPGPU_SIM_HPCL_STAT_H_
#define GPGPU_SIM_HPCL_STAT_H_

#include <string>

class hpcl_stat {
public:
  hpcl_stat();
  virtual ~hpcl_stat();

private:
  unsigned long long _num_samples;
  double _sample_sum;
  double _min;
  double _max;
  std::string _name;

public:
  hpcl_stat(const std::string name);

  void clear();
  double avg() const;
  double max() const;
  double min() const;
  double sum() const;
  void add_sample(double val);

  //added by kh(041616)
  double get_sample_no();
  //
};

#endif /* GPGPU_SIM_HPCL_STAT_H_ */
