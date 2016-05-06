/*
 * hpcl_virt_flit.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_DICT_CTRL_MSG_H_
#define HPCL_DICT_CTRL_MSG_H_

#include <vector>

template<class K>
class hpcl_dict_ctrl_msg {
public:
  hpcl_dict_ctrl_msg()
  {
    m_id=next_ctrl_uid++;
  }
  virtual ~hpcl_dict_ctrl_msg() {}

public:
  enum DICT_CTRL_MSG_TYPE {
    /*Private Table*/
    HPCL_PRIVATE_UPDATE = 0,

    /*Shared Table*/
    HPCL_SHARED_INVALIDATE_REQ,
    HPCL_SHARED_INVALIDATE_ACK,
    HPCL_SHARED_INVALIDATE_REPLACE,
    HPCL_SHARED_UPDATE,
  };

private:
  enum DICT_CTRL_MSG_TYPE m_type;
  K m_victim_word;
  K m_new_word;

public:
  K get_victim_word()				{	return m_victim_word;	}
  K get_new_word()				{	return m_new_word;	}
  enum DICT_CTRL_MSG_TYPE get_type()		{	return m_type;		}

  void set_victim_word(K word)			{	m_victim_word = word;	}
  void set_new_word(K word)			{	m_new_word = word;	}
  void set_type(enum DICT_CTRL_MSG_TYPE type)	{	m_type = type;	}

//added by kh(022716)
public:
  static void print_word(K word)
  {
    if(sizeof(K) == sizeof(unsigned char))		printf("%02x", word);
    else if(sizeof(K) == sizeof(unsigned short))	printf("%04x", word);
    else if(sizeof(K) == sizeof(unsigned int)) 		printf("%08x", word);
    else if(sizeof(K) == sizeof(unsigned long long))    printf("%016llx", word);
  }

  void print()
  {
    printf("dict_ctrl_msg %d | type %d", m_id, m_type);
    if(m_type == HPCL_PRIVATE_UPDATE) {
	printf(" word ");
	print_word(m_new_word);
	printf(" dest %d", m_dest_node);
	printf("\n");
    } else{

    }
  }


private:
  int m_dest_node;

public:
  void set_dest_node(int dest_node)		{	m_dest_node = dest_node;}
  int get_dest_node()				{	return m_dest_node;	  }


//added by kh(022916)
private:
  int m_word_index;	//index in the dictionary

public:
  void set_word_index(int word_index)		{	m_word_index = word_index;}
  int get_word_index()				{	return m_word_index;	  }
///

public:
  static unsigned next_ctrl_uid;
private:
  unsigned m_id;
public:
  unsigned get_id()				{	return m_id;		  }



//added by kh(030216)
private:
  int m_src_node;

public:
  void set_src_node(int src_node)		{	m_src_node = src_node;}
  int get_src_node()				{	return m_src_node;	  }



};

template<class K>
unsigned hpcl_dict_ctrl_msg<K>::next_ctrl_uid=1;

#endif /* HPCL_DICT_CTRL_MSG_H_ */
