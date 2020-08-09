./benchmark -o out -b resources/instances -m 2000 \
	-i random_small_int \
	-i random_small_float \
	-i random_large_float \
	-i AGPLIB/AGP2007/agp2007-fat \
	-i AGPLIB/AGP2007/agp2007-minarea \
	-i AGPLIB/AGP2007/agp2007-orthorand \
	-i AGPLIB/AGP2008a/agp2008a-cvk \
	-i AGPLIB/AGP2008a/agp2008a-fat \
	-i AGPLIB/AGP2008a/agp2008a-orthorand \
	-i AGPLIB/AGP2008a/agp2008a-rvk \
	-i AGPLIB/AGP2008b/agp2008b-cvk \
	-i AGPLIB/AGP2008b/agp2008b-orthorand \
	-i AGPLIB/AGP2008b/agp2008b-simplerand \
	-i AGPLIB/AGP2009a/agp2009a-cvk \
	-i AGPLIB/AGP2009a/agp2009a-orthorand \
	-i AGPLIB/AGP2009a/agp2009a-rvk \
	-i AGPLIB/AGP2009a/agp2009a-simplerand \
	2>&1 | tee benchmark_all_result.txt

