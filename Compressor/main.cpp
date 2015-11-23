#include "compress.h"
#include <iomanip>

int main() {
	std::string test = "$aac$ff$fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff$";
	/*std::string test = "Often times, bugs are migrated into the developer's database that may contain hundreds, """
		"""if not thousands, of other issues. Imagine trying to search this database for \"navigation bar\". """
		"""That search will return every issue related to the navigation bar. Searching for \"wrapping to second line\" """
		"""is much more specific making it easier to find the bug. Your bug report needs to survive (and be useful) """
		"""beyond the current test cycle; a strong title will help it through it's journey.";*/

	std::cout << "Test:\t" << test << "\n\n";

	Compressor comp;
	
	std::string compressed = comp.compress(test, _compression_alg::RLE);
	
	std::cout << "Compressed:\t" << compressed << "\n\n";
	
	std::string decompressed = comp.decompress(compressed, _compression_alg::RLE);
	
	std::cout << "Decompressed:\t" << decompressed << "\n\n";
	
	std::cout << std::setprecision(3) << "\nCheck: " << ((decompressed == test) ? "OK!" : "ERROR!")
		<< "\nCompression: " << (float(compressed.length()) / test.length() * 100.0f) << "%\n\n";
	
	system("pause");
	return 0;
}