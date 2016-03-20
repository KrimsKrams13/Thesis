echo "Murmur Uni"
gnuplot -e "hash='Murmur'" -e "test='uniform'" gnuplot_distribution
echo "Murmur Gauss"
gnuplot -e "hash='Murmur'" -e "test='gaussian'" gnuplot_distribution
echo "Murmur Exp"
gnuplot -e "hash='Murmur'" -e "test='exponential'" gnuplot_distribution
echo "Tabulation Uni"
gnuplot -e "hash='Tabulation'" -e "test='uniform'" gnuplot_distribution
echo "Tabulation Gauss"
gnuplot -e "hash='Tabulation'" -e "test='gaussian'" gnuplot_distribution
echo "Tabulation Exp"
gnuplot -e "hash='Tabulation'" -e "test='exponential'" gnuplot_distribution
