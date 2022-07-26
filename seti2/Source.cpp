#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <locale.h>	
#include <stdint.h>
//#include <windows.h>

struct FourOctets {
	bool isValid;
	int octets[ 4 ];
};

class UserException {
public:
	UserException() {
		
	}
};

//разбивает входящую строку по актетам
FourOctets GetOctets( const std::string & textLine ) {
	// конструируем исходные данные
	std::string testLine( textLine );

	// заводим структуру октетов 
	FourOctets answer;
	answer.isValid = false;
	for(int i=0; i<4; i++) {
		answer.octets[i] = -1;
	}

	// если встретили символ - выходим
	for (int i=0; i < testLine.length(); i++) {
		if (isalpha((unsigned char)testLine[i])) {
				return answer;
		}
	}

	// разбиваем по точкам
	size_t foundOffset = 0;
	size_t numberOfDots = 0;
	while ( ( foundOffset = testLine.find( '.' ) ) != std::string::npos ) { 
		testLine.replace( foundOffset, 1, 1, ' ' );
		numberOfDots++;
	}

	// валидация по актетам
	if( numberOfDots == 3 ) {
		std::stringstream ss( testLine );
		ss.exceptions( std::ios::failbit | std::ios::badbit );
		int *octets = answer.octets;
		try {
			ss >> octets[ 0 ] >> octets[ 1 ] >> octets[ 2 ] >> octets[ 3 ];
			for(int i=0; i<4; i++) {
				if(octets[i] > 255 || octets[i] < 0) {
					throw UserException();
				}
			}
			answer.isValid = true;
		} catch (...) {
			answer.isValid = false;
		}
	}
	return answer;
}

// формируем ip-aдрес по входной строке
FourOctets GetIpAddress(std::string ip) {
	FourOctets ipAddress = GetOctets(ip);
	return ipAddress;
}

// формируем маску по входной строке
FourOctets GetMask(std::string mask) {
	int posValues[] = {0, 128, 192, 224, 240, 248, 252, 254, 255};
	int posValuesCounter = 9;
	bool flag = false;
	int counter = 0;

	// создаем маску
	FourOctets ipMask = GetOctets(mask);
	if(ipMask.isValid = false) {
		return ipMask;
	}

	// первый октет маски - нулевой => вся маска нулевая
	if(ipMask.octets[0] == 0) {
		if(ipMask.octets[1] !=0 || ipMask.octets[2] !=0 || ipMask.octets[3] !=0) {
			ipMask.isValid = false;
			return ipMask;
		}
	}

	// проверяем, что каждый октет маски принимает возможные значения
	for(int i=0; i<4; i++) {
		for (int j=0; j<posValuesCounter; j++) {
			if (ipMask.octets[i] == posValues[j]) {
				counter++;
				break;
			}
		}
	}
	if (counter == 4) {
			ipMask.isValid = true;
	} else {
			ipMask.isValid = false;
			return ipMask;
	}
	counter = 0;

	try {
		for (int i=0; i<3; i++) {

			// каждый последующий не больше предыдущего
			if(ipMask.octets[i+1] > ipMask.octets[i]) {
				throw UserException();
			}

			// если есть последующий октет, то предыдущий - 255
			if(ipMask.octets[i+1] != 0){
				if(ipMask.octets[i] != 255){
					throw UserException();
				}
			}

			// если все тесты прошли
			ipMask.isValid = true;

		}
	} catch (...) {
		ipMask.isValid = false;
	}
	return ipMask;
}

// получить хостовую часть
FourOctets GetHostPart(FourOctets& address, FourOctets& mask) {
	FourOctets hostPart;
	for(int i=0; i<4; i++) {
		hostPart.octets[i] = address.octets[i] & (255 - mask.octets[i]);
	}
	return hostPart;
}

// получить сетевую часть
FourOctets GetWebPart(FourOctets& address, FourOctets& mask) {
	FourOctets webPart;
	for(int i=0; i<4; i++) {
		webPart.octets[i] = address.octets[i] & mask.octets[i];
	}
	return webPart;
}

// сравнить значения октетов. все совпали - true, хотя бы одно различие - false
bool OctetsAreEqual(FourOctets in1, FourOctets in2) {
	int counter = 0;
	for(int i=0; i<4; i++) {
		if(in1.octets[i] == in2.octets[i]) {
			counter++;
		}
	}
	if(counter == 4) 
		return true;
	else return false;
}

// получить шлюз по умолчанию
FourOctets GetDefaultGateway(FourOctets ipAddress, FourOctets ipMask) {
	FourOctets dg;
	int counter = -1;

	// работаем с сетевой частью
	dg = GetWebPart(ipAddress, ipMask);

	// вычисляем последний октет, не равный 255 (к которому можно прибавить 1)
	for(int i=3; i>=0; i--) {
		if(dg.octets[i] < 255) {
			counter = i;
			break;
		}
	}

	// если такой октет существует, то увеличиваем его на 1
	// а все последующие октеты зануляем (перенос разряда)
	if(counter > 0) {
		dg.isValid = true;
		dg.octets[counter] += 1;
		if(counter < 3) {
			for(int i=counter+1; i<4; i++) {
				dg.octets[i] = 0;
			}
		}
	} else {
		dg.isValid = false;
	}

	// нужна ли проверка что ip адрес != шлюзу?
	/*if(OctetsAreEqual(dg, ipAddress)) {
		counter = -1;
		for(int i=3; i>=0; i--) {
			if(dg.octets[i] < 255) {
				counter = i;
				break;
			}
		}
		if(counter > 0) {
			dg.isValid = true;
			dg.octets[counter] += 1;
			if(counter < 3) {
				for(int i=counter+1; i<4; i++) {
					dg.octets[i] = 0;
				}
			}
		} else {
			dg.isValid = false;
		}
	}*/
	return dg;
}

// получить широковещательный адрес
FourOctets GetBroadcastIp(FourOctets ipAddress, FourOctets ipMask) {
	FourOctets bcIp;
	size_t temp = 0;
	for (int i=0; i<4; i++) {
		bcIp.octets[i] = ipAddress.octets[i] | (255 - ipMask.octets[i]);
	}
	return bcIp;
}

// подсчитать количество хостов в подсети
int CountNumberOfHosts(FourOctets address, FourOctets mask) {
	int noh = 0;
	FourOctets broadcast = GetBroadcastIp(address, mask);
	FourOctets web = GetWebPart(address, mask);
	for(int i=0; i<4; i++) {
		noh += broadcast.octets[i] - web.octets[i];
	}
	return noh;
}

// проверить существование сети по маске и адресу
void ValidateWeb(FourOctets& address, FourOctets& mask) {
	int counter = 0;
	FourOctets hostPart;
	try {
		
		// сеть слишком мала
		if(CountNumberOfHosts(address, mask) <= 2) {
			throw UserException();
		}

		// ip != широковещательному
		FourOctets tempBcIp = GetBroadcastIp(address, mask);
		if(OctetsAreEqual(address, tempBcIp)) {
			throw UserException();
		}

		// ip адрес != сетевой части
		FourOctets webPart = GetWebPart(address, mask);
		if(OctetsAreEqual(address, webPart)) {
			throw UserException();
		}

		//// ip адрес != шлюз (вызывает ошибку на 3 тесте!)
		//FourOctets tempDg = GetDefaultGateway(address, mask);
		//if(OctetsAreEqual(address, tempDg)) {
		//	throw UserException();
		//}

		// если все тесты прошли
		mask.isValid = true;
		address.isValid = true;

	} catch (...) {
		mask.isValid = false;
		address.isValid = false;
	}
	return;
}


// делит входную строку на 2ip - адреса по пробелу, оба из них проходят валидацию, 
// + дополнительная проверка существования сети по ip-адресу и маске
bool SplitStrToIpAndMaskBySpace(std::string src, FourOctets& ipAddress, FourOctets& ipMask) {

	// считаем пробелы во входной строке
	size_t targetSpaceOffset = 0;
	size_t curSpaceOffset = 0;
	size_t numOfSpaces = 0;

	while ( (curSpaceOffset = src.find( ' ' )) != std::string::npos ) {
			if (curSpaceOffset != std::string::npos) {
				targetSpaceOffset = curSpaceOffset;
			}
			src.replace(curSpaceOffset, 1, 1, '_');
			numOfSpaces++;
	}

	// валидация по кол-ву пробелов
	if (numOfSpaces != 1) {
			ipAddress.isValid = false;
			ipMask.isValid = false;
			return false;
	} else {
		// формируем ip-адрес
		ipAddress = GetIpAddress(src.substr(0, targetSpaceOffset));

		// формируем маску
		if(ipAddress.isValid = true) {
			ipMask = GetMask(src.substr(targetSpaceOffset+=1, src.length()));
		} else {
			return false;
		}

		// валидация сети по маске и адресу
		if(ipAddress.isValid && ipMask.isValid) {
			ValidateWeb(ipAddress, ipMask);
		}
		return ipAddress.isValid && ipMask.isValid;
	}
}



// выводит октеты ip-адреса через точку
void PrintAddress(FourOctets address) {
	for (int i=0; i<4; i++) {
		std::cout << address.octets[i];
		if (i != 3) {
			std::cout << ".";
		}
		if (i == 3) {
			std::cout << std::endl;
		}
	}
}

int main(void) {
	setlocale(LC_ALL, "RU");
	std::string inputString;
	FourOctets ipAddress;
	FourOctets ipMask;
	FourOctets dg;
	/*std::stringbuf strBuf("172.20.10.71 255.255.0.0");
	std::istream tStr(&strBuf);*/
	while( getline (/*tStr*/ std::cin, inputString) ) {
		// разделить строку по пробелу на 2 части. если частей больше - выход.
		SplitStrToIpAndMaskBySpace(inputString, ipAddress, ipMask);
		try {
			if( !ipAddress.isValid || !ipMask.isValid) {
				throw UserException();
			}
			// получаем адрес шлюза по умолчанию
			dg = GetDefaultGateway(ipAddress, ipMask);
			if( !dg.isValid) {
				throw UserException();
			} else {
				// если все удачно - печатаем адрес шлюза
				PrintAddress(dg);
			}
		} catch (UserException) {
			std::cout << "X" << std::endl;
		}
	}
	return 0;
}