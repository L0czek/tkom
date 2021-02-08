#ifndef __LOCALE_HPP__
#define __LOCALE_HPP__

#include <unistd.h>
#include <string>
#include <locale>

class Locale {
	std::string cpp_name;
	std::locale cpp_locale;
	Locale(const char *name) : cpp_name(name), cpp_locale{ name }
	{
		std::locale::global(cpp_locale);
	}

    public:
	static Locale get()
	{
		static Locale loc{ "en_US.UTF8" };
		return loc;
	}

	const std::string &name() const noexcept
	{
		return cpp_name;
	}
	std::locale locale() const noexcept
	{
		return cpp_locale;
	}
};

#endif
