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

//��������� �������� ������ �� �������
FourOctets GetOctets( const std::string & textLine ) {
	// ������������ �������� ������
	std::string testLine( textLine );

	// ������� ��������� ������� 
	FourOctets answer;
	answer.isValid = false;
	for(int i=0; i<4; i++) {
		answer.octets[i] = -1;
	}

	// ���� ��������� ������ - �������
	for (int i=0; i < testLine.length(); i++) {
		if (isalpha((unsigned char)testLine[i])) {
				return answer;
		}
	}

	// ��������� �� ������
	size_t foundOffset = 0;
	size_t numberOfDots = 0;
	while ( ( foundOffset = testLine.find( '.' ) ) != std::string::npos ) { 
		testLine.replace( foundOffset, 1, 1, ' ' );
		numberOfDots++;
	}

	// ��������� �� �������
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

// ��������� ip-a���� �� ������� ������
FourOctets GetIpAddress(std::string ip) {
	FourOctets ipAddress = GetOctets(ip);
	return ipAddress;
}

// ��������� ����� �� ������� ������
FourOctets GetMask(std::string mask) {
	int posValues[] = {0, 128, 192, 224, 240, 248, 252, 254, 255};
	int posValuesCounter = 9;
	bool flag = false;
	int counter = 0;

	// ������� �����
	FourOctets ipMask = GetOctets(mask);
	if(ipMask.isValid = false) {
		return ipMask;
	}

	// ������ ����� ����� - ������� => ��� ����� �������
	if(ipMask.octets[0] == 0) {
		if(ipMask.octets[1] !=0 || ipMask.octets[2] !=0 || ipMask.octets[3] !=0) {
			ipMask.isValid = false;
			return ipMask;
		}
	}

	// ���������, ��� ������ ����� ����� ��������� ��������� ��������
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

			// ������ ����������� �� ������ �����������
			if(ipMask.octets[i+1] > ipMask.octets[i]) {
				throw UserException();
			}

			// ���� ���� ����������� �����, �� ���������� - 255
			if(ipMask.octets[i+1] != 0){
				if(ipMask.octets[i] != 255){
					throw UserException();
				}
			}

			// ���� ��� ����� ������
			ipMask.isValid = true;

		}
	} catch (...) {
		ipMask.isValid = false;
	}
	return ipMask;
}

// �������� �������� �����
FourOctets GetHostPart(FourOctets& address, FourOctets& mask) {
	FourOctets hostPart;
	for(int i=0; i<4; i++) {
		hostPart.octets[i] = address.octets[i] & (255 - mask.octets[i]);
	}
	return hostPart;
}

// �������� ������� �����
FourOctets GetWebPart(FourOctets& address, FourOctets& mask) {
	FourOctets webPart;
	for(int i=0; i<4; i++) {
		webPart.octets[i] = address.octets[i] & mask.octets[i];
	}
	return webPart;
}

// �������� �������� �������. ��� ������� - true, ���� �� ���� �������� - false
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

// �������� ���� �� ���������
FourOctets GetDefaultGateway(FourOctets ipAddress, FourOctets ipMask) {
	FourOctets dg;
	int counter = -1;

	// �������� � ������� ������
	dg = GetWebPart(ipAddress, ipMask);

	// ��������� ��������� �����, �� ������ 255 (� �������� ����� ��������� 1)
	for(int i=3; i>=0; i--) {
		if(dg.octets[i] < 255) {
			counter = i;
			break;
		}
	}

	// ���� ����� ����� ����������, �� ����������� ��� �� 1
	// � ��� ����������� ������ �������� (������� �������)
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

	// ����� �� �������� ��� ip ����� != �����?
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

// �������� ����������������� �����
FourOctets GetBroadcastIp(FourOctets ipAddress, FourOctets ipMask) {
	FourOctets bcIp;
	size_t temp = 0;
	for (int i=0; i<4; i++) {
		bcIp.octets[i] = ipAddress.octets[i] | (255 - ipMask.octets[i]);
	}
	return bcIp;
}

// ���������� ���������� ������ � �������
int CountNumberOfHosts(FourOctets address, FourOctets mask) {
	int noh = 0;
	FourOctets broadcast = GetBroadcastIp(address, mask);
	FourOctets web = GetWebPart(address, mask);
	for(int i=0; i<4; i++) {
		noh += broadcast.octets[i] - web.octets[i];
	}
	return noh;
}

// ��������� ������������� ���� �� ����� � ������
void ValidateWeb(FourOctets& address, FourOctets& mask) {
	int counter = 0;
	FourOctets hostPart;
	try {
		
		// ���� ������� ����
		if(CountNumberOfHosts(address, mask) <= 2) {
			throw UserException();
		}

		// ip != ������������������
		FourOctets tempBcIp = GetBroadcastIp(address, mask);
		if(OctetsAreEqual(address, tempBcIp)) {
			throw UserException();
		}

		// ip ����� != ������� �����
		FourOctets webPart = GetWebPart(address, mask);
		if(OctetsAreEqual(address, webPart)) {
			throw UserException();
		}

		//// ip ����� != ���� (�������� ������ �� 3 �����!)
		//FourOctets tempDg = GetDefaultGateway(address, mask);
		//if(OctetsAreEqual(address, tempDg)) {
		//	throw UserException();
		//}

		// ���� ��� ����� ������
		mask.isValid = true;
		address.isValid = true;

	} catch (...) {
		mask.isValid = false;
		address.isValid = false;
	}
	return;
}


// ����� ������� ������ �� 2ip - ������ �� �������, ��� �� ��� �������� ���������, 
// + �������������� �������� ������������� ���� �� ip-������ � �����
bool SplitStrToIpAndMaskBySpace(std::string src, FourOctets& ipAddress, FourOctets& ipMask) {

	// ������� ������� �� ������� ������
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

	// ��������� �� ���-�� ��������
	if (numOfSpaces != 1) {
			ipAddress.isValid = false;
			ipMask.isValid = false;
			return false;
	} else {
		// ��������� ip-�����
		ipAddress = GetIpAddress(src.substr(0, targetSpaceOffset));

		// ��������� �����
		if(ipAddress.isValid = true) {
			ipMask = GetMask(src.substr(targetSpaceOffset+=1, src.length()));
		} else {
			return false;
		}

		// ��������� ���� �� ����� � ������
		if(ipAddress.isValid && ipMask.isValid) {
			ValidateWeb(ipAddress, ipMask);
		}
		return ipAddress.isValid && ipMask.isValid;
	}
}



// ������� ������ ip-������ ����� �����
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
		// ��������� ������ �� ������� �� 2 �����. ���� ������ ������ - �����.
		SplitStrToIpAndMaskBySpace(inputString, ipAddress, ipMask);
		try {
			if( !ipAddress.isValid || !ipMask.isValid) {
				throw UserException();
			}
			// �������� ����� ����� �� ���������
			dg = GetDefaultGateway(ipAddress, ipMask);
			if( !dg.isValid) {
				throw UserException();
			} else {
				// ���� ��� ������ - �������� ����� �����
				PrintAddress(dg);
			}
		} catch (UserException) {
			std::cout << "X" << std::endl;
		}
	}
	return 0;
}