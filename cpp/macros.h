#ifndef __MACROS_H__
#define __MACROS_H__

#define REPEAT_0(__DO) {}
#define REPEAT_1(__DO) {__DO;__DO;}

#define SERAIL(WORKS) \
	friend class boost::serialization::access;		\
	template<typename Archive> 						\
	void serialize(Archive & ar, const unsigned int file_version) {	\
		do{WORKS;}while(false);	\
	}
#define EMPTY_SERIAL() 	SERAIL()
#define EMPTY_SERIAL_BASE(BASE_CLASS) SERAIL(ar & boost::serialization::base_object<BASE_CLASS>(*this);)
#define SERIAL_ARG_1(ARG_1) SERAIL(ar&ARG_1;)
#define SERIAL_ARG_2(ARG_1,ARG_2) SERAIL(ar&ARG_1;ar&ARG_2)
#define SERIAL_ARG_3(ARG_1,ARG_2,ARG_3) SERAIL(ar&ARG_1;ar&ARG_2;ar&ARG_3)
#define SERIAL_ARG_4(ARG_1,ARG_2,ARG_3,ARG_4) SERAIL(ar&ARG_1;ar&ARG_2;ar&ARG_3;ar&ARG_4)
#define SERIAL_ARG_5(ARG_1,ARG_2,ARG_3,ARG_4,ARG_5) SERAIL(ar&ARG_1;ar&ARG_2;ar&ARG_3;ar&ARG_4;ar&ARG_5;)
#define SERIAL_ARG_6(ARG_1,ARG_2,ARG_3,ARG_4,ARG_5,ARG_6) SERAIL(ar&ARG_1;ar&ARG_2;ar&ARG_3;ar&ARG_4;ar&ARG_5;ar&ARG_6;)



#endif
