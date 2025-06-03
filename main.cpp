#include <iostream>
#include <sstream>

auto get_compiler_info() {
    std::stringstream str;

#if defined(__clang__)
    str << "clang " << __clang_major__ << '.' << __clang_minor__ << '.' << __clang_patchlevel__;
#elif defined(__GNUC__) && !defined(__ICC)
    str << "gcc " << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
#elif defined(_MSC_VER)
    str << "msvc " << _MSC_VER;
#elif defined(__ICC)
    str << "icc " << __VERSION__;
#endif

    return str.str();
}

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    // TIP Press <shortcut actionId="RenameElement"/> when your caret is at the
    // <b>lang</b> variable name to see how CLion can help you rename it.
    const auto lang = "C++";
    std::cout << "Hello and welcome to " << lang << "!\n";

    std::cout << get_compiler_info() << '\n';
    return 0;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.
