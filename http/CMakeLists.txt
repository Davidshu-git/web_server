# 设定源文件变量
set(HTTP_SRCS
    HttpContext.cc
    HttpResponse.cc
    HttpServer.cc
)

# 生成http_lib库
add_library(http_lib ${HTTP_SRCS})

# 添加链接库
target_link_libraries(http_lib net_lib)