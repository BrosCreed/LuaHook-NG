#pragma once
#include <iomanip>

namespace Olipro
{
	class ProcessSearcher {
		LPBYTE moduleBase;
		size_t moduleSize;

	public:
		ProcessSearcher()
		{
			moduleBase = reinterpret_cast<LPBYTE>(GetModuleHandle(nullptr));

			if (moduleBase)
			{
				auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(
					moduleBase);

				if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
				{
					auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
						moduleBase + dosHeader->e_lfanew);
					moduleBase = moduleBase +
						ntHeaders->OptionalHeader.BaseOfCode;
					moduleSize = ntHeaders->OptionalHeader.SizeOfCode - 1;
				}
			}
		}

		template <typename T>
		T FindFunction(	T& obj, const std::string& signature,
						const std::string& predicate)
		{
			auto needle = reinterpret_cast<const BYTE* const>(
				signature.c_str());
			auto predFunc = [w = predicate[0]](const BYTE& a, const BYTE& b) {
				return b == w || a == b;
			};
			auto result = std::search(moduleBase, moduleBase + moduleSize,
				needle, needle + signature.length(), predFunc);
			obj = reinterpret_cast<T>(result != (moduleBase + moduleSize) ?
				result : nullptr);
			return obj;
		}

		template <typename T>
		T FindFunction(T& obj, const std::string& signature)
		{
			auto needle = reinterpret_cast<const BYTE* const>(
				signature.c_str());
			auto result = std::search(moduleBase, moduleBase + moduleSize,
				needle, needle + signature.length());
			obj = reinterpret_cast<T>(result != (moduleBase + moduleSize) ?
				result : nullptr);
			return obj;
		}

		template <typename T>
		T& FindFunction(const std::string& signature)
		{
			auto needle = reinterpret_cast<const BYTE* const>(
				signature.c_str());
			auto result = std::search(moduleBase, moduleBase + moduleSize,
				needle, needle + signature.length());
			auto ret = result != (moduleBase + moduleSize) ? result : nullptr;
			if (ret == nullptr) {
				std::ofstream file{ "missingsigs.txt", std::ios::app };
				file << "not found: " << typeid(T).name();
				for (auto i : signature)
					file << std::hex << std::setfill('0') << std::setw(2)
					<< static_cast<unsigned int>(static_cast<unsigned char>(i)) << " ";
				file << "\n";
			}
			return *reinterpret_cast<std::add_pointer_t<T>>(ret);
		}
	};
}
