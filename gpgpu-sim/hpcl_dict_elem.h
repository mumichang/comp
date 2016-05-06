/*
 * hpcl_dict_elem.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_DICT_ELEM_H_
#define HPCL_DICT_ELEM_H_

template<class K>
class hpcl_dict_elem {
/*
public:
	hpcl_dict_elem();
	virtual ~hpcl_dict_elem();
*/

private:
  K m_word;

public:
  hpcl_dict_elem (K word, unsigned long long time);
  virtual ~hpcl_dict_elem ();

  unsigned long long m_last_use_time;	//for LRU
  unsigned int m_freq;			//for LFU

public:
  void print_word();
  void print();
  bool search(K word);
  void update_last_use_time(unsigned long long time);
  void update_freq();
  unsigned long long get_last_use_time();
  unsigned int get_freq();
  void init(K word, unsigned long long time);
  static void print_word_data(K word) {
    if(sizeof(K) == sizeof(unsigned char)) {
      printf("%02x", word);
    } else if(sizeof(K) == sizeof(unsigned short)) {
      printf("%04x", word);
    } else if(sizeof(K) == sizeof(unsigned int)) {
      printf("%08x", word);
    } else if(sizeof(K) == sizeof(unsigned long long)) {
      printf("%016llx", word);
    }
  }
  K get_word();


//added by kh(030216)
private:
  bool m_valid;

public:
  bool get_valid()	{	return m_valid;	}
  void set_valid()	{	m_valid = true;	}
  void clear_valid()	{	m_valid = false;}
///

//added by kh(030816)
private:
  bool m_hold_flag;	 //for LFU

public:
  bool get_hold_flag()		{	return m_hold_flag;	}
  void set_hold_flag()		{	m_hold_flag = true;	}
  void clear_hold_flag()	{	m_hold_flag = false;	}
///

};


template<class K>
hpcl_dict_elem<K>::hpcl_dict_elem (K word, unsigned long long time) {
  init(word, time);
}

template<class K>
hpcl_dict_elem<K>::~hpcl_dict_elem() {
  // TODO Auto-generated destructor stub
}

template<class K>
void hpcl_dict_elem<K>::print_word()
{
  if(sizeof(K) == sizeof(unsigned char)) {
	  printf("%02x", m_word);
  } else if(sizeof(K) == sizeof(unsigned short)) {
	  printf("%04x", m_word);
  } else if(sizeof(K) == sizeof(unsigned int)) {
	  printf("%08x", m_word);
  } else if(sizeof(K) == sizeof(unsigned long long)) {
	  printf("%016llx", m_word);
  }
}

template<class K>
void hpcl_dict_elem<K>::print()
{
  if(m_valid)	printf("v | ");
  else 		printf("i | ");

  printf("word = ");
  print_word();
  printf(" time = %llu, freq = %u", m_last_use_time, m_freq);

}

template<class K>
bool hpcl_dict_elem<K>::search(K word) {
  return (m_word == word)? true: false;
}

template<class K>
void hpcl_dict_elem<K>::update_last_use_time(unsigned long long time) {
	m_last_use_time = time;
}

template<class K>
void hpcl_dict_elem<K>::update_freq() {
	m_freq++;
}

template<class K>
unsigned long long hpcl_dict_elem<K>::get_last_use_time() {
	return m_last_use_time;
}

template<class K>
unsigned int hpcl_dict_elem<K>::get_freq() {
	return m_freq;
}

template<class K>
void hpcl_dict_elem<K>::init(K word, unsigned long long time) {
  m_last_use_time = time;
  m_freq = 1;		//changed from 0 to 1 by kh(042116)
  m_word = word;
  m_valid = false;
  m_hold_flag = false;
}

template<class K>
K hpcl_dict_elem<K>::get_word() {
	return m_word;
}

#endif /* HPCL_DICT_ELEM_H_ */
