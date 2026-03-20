.PHONY: test prod

test:
	@mkdir -p build_test
	@cmake -S . -B build_test -DBOT_NAME=axuaxi_test
	@cmake --build build_test
	@echo "BUILT TEST"

prod:
	@mkdir -p build_prod
	@cmake -S . -B build_prod -DBOT_NAME=axuaxi_prod
	@cmake --build build_prod
	@echo "BUILT PROD"