#ifndef BFD4C357_3666_43C9_BF9D_1F22AC39B611
#define BFD4C357_3666_43C9_BF9D_1F22AC39B611

#include <string>

class ResourcePath
{
  public:
    ResourcePath() = default;
    ~ResourcePath() = default;

    static void init();
    static std::string resolve(const char *relativePath);
    static std::string resolve(const std::string &relativePath);

  private:
    static std::string s_basePath;
};

#endif /* BFD4C357_3666_43C9_BF9D_1F22AC39B611 */
