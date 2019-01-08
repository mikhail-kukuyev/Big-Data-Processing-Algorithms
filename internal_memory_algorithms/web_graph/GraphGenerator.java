import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.IntStream;
import java.util.stream.Stream;

public class GraphGenerator {

    private int n;
    private int m;

    private List<Set<Integer>> graph;
    private int[] degrees;
    private Set<BitSet> components;
    private int[][] distances;
    private double avgDistance;
    private int diameter;
    private int[] degreeDistribution;

    public static void main(String[] args) throws Exception {

        int diameterSum = 0;
        double avgDistanceSum = 0;
        int N = 100;
        int degreesCount = 100;
        double[] degreeDistributionSum = new double[degreesCount];

        for (int i = 1; i < N+1; i++) {
            GraphGenerator graphGenerator = new GraphGenerator();
            graphGenerator.generate(4000, 16);
            graphGenerator.calculateCharacteristics();

            diameterSum += graphGenerator.getDiameter();
            avgDistanceSum += graphGenerator.getAvgDistance();
            for (int j = 0; j < degreeDistributionSum.length; j++) {
                degreeDistributionSum[j] += graphGenerator.getDegreeDistribution()[j];
            }
            System.out.println("iteration:" + i);

            if (i % 100 == 0) {
                double factor = 1.0 / i;
                System.out.println("Diameter:");
                System.out.println(diameterSum * factor);
                System.out.println("Average distance:");
                System.out.println(avgDistanceSum * factor);
                System.out.println("Degrees:");
                for (int j = 0; j < degreeDistributionSum.length; j++) {
                    System.out.println(j + "\t" + degreeDistributionSum[j] * factor);
                }
            }
        }



        /*GraphGenerator graphGenerator = new GraphGenerator();
        graphGenerator.generate(4000, 16);
        graphGenerator.calculateCharacteristics();

        System.out.println("Graph:");
        System.out.println("Diameter: " + graphGenerator.getDiameter());
        System.out.println("Average distance:" + graphGenerator.getAvgDistance());

        System.out.println("Degrees: ");
        Arrays.stream(graphGenerator.getDegrees()).forEach(el -> System.out.print(el + ",\t"));
        System.out.println();

        System.out.println("\nDegrees distribution:");
        int[] degreeDistribution = graphGenerator.getDegreeDistribution();
        for (int i = 0; i < degreeDistribution.length; i++) {
            System.out.print(i + ": " + degreeDistribution[i] + ",\t");
        }
        System.out.println();*/
    }

    public void generate(int n, int m) {
        clearData();

        this.n = n;
        this.m = m;

        List<Set<Integer>> initGraph = Stream.generate(HashSet<Integer>::new)
                .limit(n * m)
                .collect(Collectors.toList());

        int[] initDegrees = new int[n * m];
        Random random = new Random();

        //1st step of generation
        for (int i = 0; i < n * m; i++) {
            double chance = random.nextDouble();
            double factor = 1.0 / (2 * (i + 1) - 1);
            double leftBorder;
            double rightBorder = factor;

            if (i == 0 || chance < rightBorder) {
                initDegrees[i] = 2;
                initGraph.get(i).add(i);
            } else {
                for (int j = 0; j < i; j++) {
                    leftBorder = rightBorder;
                    rightBorder += initDegrees[j] * factor;
                    if (leftBorder <= chance && chance < rightBorder) {
                        initDegrees[i] = 1;
                        initDegrees[j] += 1;
                        initGraph.get(i).add(j);
                        initGraph.get(j).add(i);
                        break;
                    }
                }
            }
        }

        //2nd step of generation
        graph = Stream.generate(HashSet<Integer>::new)
                .limit(n)
                .collect(Collectors.toList());
        degrees = new int[n];

        for (int v = 0; v < initGraph.size(); v += m) {
            Set<Integer> newAdjVertices = initGraph.get(v).stream()
                    .map(el -> el / m).collect(Collectors.toSet());
            degrees[v / m] = initDegrees[v];

            for (int nextV = v + 1; nextV < v + m; nextV++) {
                newAdjVertices.addAll(
                        initGraph.get(nextV).stream()
                                .map(el -> el / m).collect(Collectors.toSet()));

                degrees[v / m] += initDegrees[nextV];
            }
            graph.set(v / m, newAdjVertices);
        }
    }

    public void calculateCharacteristics() {
        findComponentsAndDistances();

        calculateAvgDistancesAndDiameter();

        calculateDegreeDistribution();
    }


    public Double getAvgDistance() {
        return avgDistance;
    }

    public int getDiameter() {
        return diameter;
    }

    public int[] getDegreeDistribution() {
        return degreeDistribution;
    }

    private void findComponentsAndDistances() {
        distances = new int[n][n];
        components = new HashSet<>();

        for (int i = 0; i < n; i++) {
            ArrayDeque<Integer> q = new ArrayDeque<>(n);
            q.push(i);

            BitSet used = new BitSet(n);
            used.set(i);

            while (!q.isEmpty()) {
                int v = q.pop();
                for (Integer to : graph.get(v)) {
                    if (!used.get(to)) {
                        used.set(to);
                        q.addLast(to);
                        distances[i][to] = distances[i][v] + 1;
                    }
                }
            }
            components.add(used);
        }
    }

    private void calculateDegreeDistribution() {
        degreeDistribution = new int[n * m];
        IntStream.of(degrees).forEach(degree -> degreeDistribution[degree]++);
    }

    private void calculateAvgDistancesAndDiameter() {
        int max_distance = 0;
        List<Double> avgDistances = new ArrayList<>();

        for (BitSet component : components) {
            int sum = 0;
            for (int i = 0; i < n; i++) {
                if (component.get(i)) {
                    for (int j = i + 1; j < n; j++) {
                        if (component.get(j) && max_distance < distances[i][j]) {
                            max_distance = distances[i][j];
                        }
                        sum += distances[i][j];
                    }
                }
            }
            int amount = component.cardinality();
            double avgDistance = sum * 1.0 / (amount * (amount - 1) / 2);
            avgDistances.add(avgDistance);
        }

        diameter = (components.size() == 1) ? max_distance : n;
        avgDistance = avgDistances.stream().mapToDouble(el -> el).average().getAsDouble();
    }

    private void clearData() {
        n = m = diameter = 0;
        avgDistance = 0;
        graph = null;
        degrees = null;
        degreeDistribution = null;
        components = null;
        distances = null;
    }

    public int[] getDegrees() {
        return degrees;
    }
}
