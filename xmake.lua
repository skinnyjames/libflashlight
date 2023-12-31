add_rules("mode.debug", "mode.release", "mode.valgrind")

package("libdill")
  add_deps("make")
  set_policy("package.install_always", true)
  add_urls("https://github.com/sustrik/libdill.git")
  if is_kind("static") then
    on_install("linux", "macosx", function(package)
      os.vrun("./autogen.sh")
      os.vrun("./configure %s %s --prefix=\"%s\"", "--disable-sockets", is_mode("debug") and "--enable-valgrind" or "", package:installdir())
      os.vrun("make")
      os.vrun("make check")
      os.vrun("make install")
    end)
  else
    on_install("linux", "macosx", function(package)
      os.vrun("./autogen.sh")
      os.vrun("./configure %s %s --prefix=\"%s\"", "--disable-sockets", is_mode("debug") and "--enable-valgrind" or "", package:installdir())
      os.vrun("make")
      os.vrun("make check")
      os.vrun("make install")
    end)
  end
package_end()

add_requires("libdill")
add_requires("pthread", "pcre2")

target("test")
  set_kind("binary")
  add_files("test/flashlight.c")
  add_packages("libdill", "pthread", "pcre2")
  after_build(function (target)
    os.exec("./%s", target:targetfile())
  end)

target("leaks")
  set_kind("binary")
  add_cflags("-g", "-O2", "-DDEBUG")
  add_files("test/bin.c")
  add_packages("libdill", "pthread",  "pcre2", "valgrind")
  after_build(function (target)
    os.exec("valgrind --show-leak-kinds=all --track-origins=yes --leak-check=full %s", target:targetfile())
  end)

target("lib")
  set_kind("$(kind)")
  set_basename("flashlight")
  add_files("src/flashlight.c")
  add_headerfiles("src/flashlight.h")
  add_packages("libdill", "pthread",  "pcre2")
  before_build(function (target)
    local dill = target:pkg("libdill")
    local libdill = path.join(dill:installdir(), "lib", "libdill.a")
    local libdillh = path.join(dill:installdir(), "include", "libdill.a")
    target:add("files", libdill)
    target:add("headerfiles", libdillh)
  end)
