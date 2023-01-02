cmake_minimum_required(VERSION 3.6)

install(DIRECTORY "boost/"
	DESTINATION ${INSTALL_DESTINATION}
    FILES_MATCHING PATTERN "*"
    )
