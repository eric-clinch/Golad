
#include "Tools.h"

using namespace std;

long Tools::get_time() {
	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(
		chrono::system_clock::now().time_since_epoch());
	return ms.count();
}