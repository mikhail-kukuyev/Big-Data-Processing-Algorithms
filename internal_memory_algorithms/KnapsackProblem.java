import javafx.util.Pair;

import java.util.*;

public class KnapsackProblem {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        int n = scanner.nextInt();
        int W = scanner.nextInt();

        Thing[] first = new Thing[n / 2];
        Thing[] second = new Thing[n - n / 2];
        int weight, cost;
        for (int i = 0; i < first.length; i++) {
            weight = scanner.nextInt();
            cost = scanner.nextInt();
            first[i] = new Thing(weight, cost, i + 1);
        }
        for (int i = 0; i < second.length; i++) {
            weight = scanner.nextInt();
            cost = scanner.nextInt();
            second[i] = new Thing(weight, cost, i + n / 2 + 1);
        }
        scanner.close();

        NavigableSet<Knapsack> firstSet = createKnapsackNavigableSet(first, W);
        List<Knapsack> secondList = createKnapsackList(second, W);

        first = null;
        second = null;

        Pair<Knapsack, Knapsack> bestKnapsacks = null;
        long bestCost = 0;
        long sumCost;
        for (Knapsack secondKnapsack : secondList) {
            Knapsack newKnapsack = new Knapsack();
            newKnapsack.setTotalWeight(W - secondKnapsack.getTotalWeight());
            Knapsack firstKnapsack = firstSet.floor(newKnapsack);

            sumCost = firstKnapsack.getTotalCost() + secondKnapsack.getTotalCost();
            if (bestCost < sumCost) {
                bestKnapsacks = new Pair<>(firstKnapsack, secondKnapsack);
                bestCost = sumCost;
            }
        }


        List<Integer> thingNumbers = bestKnapsacks.getKey().getThingNumbers();
        thingNumbers.addAll(bestKnapsacks.getValue().getThingNumbers());

        System.out.println(thingNumbers.size());
        thingNumbers.stream()
                .sorted()
                .forEach(el -> System.out.print(el + " "));
    }

    private static List<Knapsack> createKnapsackList(Thing[] things, int W) {
        int N = things.length;

        List<Knapsack> result = new ArrayList<>();
        result.add(new Knapsack());

        for (int mask = 0; mask < (1 << N); mask++) {
            Knapsack knapsack = new Knapsack();
            for (int j = 0; j < N; j++) {
                if ((mask & (1 << j)) != 0) {
                    knapsack.add(things[j]);
                }
            }
            if (knapsack.getTotalWeight() <= W) {
                result.add(knapsack);
            }
        }
        return result;
    }

    private static NavigableSet<Knapsack> createKnapsackNavigableSet(Thing[] things, int W) {
        int N = things.length;

        NavigableSet<Knapsack> result = new TreeSet<>();

        result.add(new Knapsack());

        for (int mask = 0; mask < (1 << N); mask++) {
            Knapsack knapsack = new Knapsack();
            for (int j = 0; j < N; j++) {
                if ((mask & (1 << j)) != 0) {
                    knapsack.add(things[j]);
                }
            }
            if (knapsack.getTotalWeight() <= W) {
                Knapsack floorKnapsack = result.floor(knapsack);
                if (floorKnapsack == null) {
                    result.add(knapsack);
                }
                else if (floorKnapsack.getTotalCost() < knapsack.getTotalCost()) {
                    if (floorKnapsack.getTotalWeight() == knapsack.getTotalWeight()) {
                        result.remove(floorKnapsack);
                    }
                    result.add(knapsack);
                }

                Knapsack higherKnapsack = result.higher(knapsack);
                while (higherKnapsack != null) {
                    if (knapsack.getTotalCost() >= higherKnapsack.getTotalCost()) {
                        result.remove(higherKnapsack);
                    }
                    else {
                        break;
                    }
                    higherKnapsack = result.higher(knapsack);
                }
            }
        }
        return result;
    }
}

class Knapsack implements Comparable<Knapsack>{

    long totalWeight = 0;
    long totalCost = 0;
    List<Integer> thingNumbers = new ArrayList<>();

    public Knapsack() {
    }

    public void add(Thing thing) {
        thingNumbers.add(thing.getNumber());
        totalWeight += thing.getWeight();
        totalCost += thing.getCost();
    }

    public long getTotalWeight() {
        return totalWeight;
    }

    public long getTotalCost() {
        return totalCost;
    }

    public List<Integer> getThingNumbers() {
        return thingNumbers;
    }

    @Override
    public int compareTo(Knapsack knapsack) {
        if (totalWeight != knapsack.getTotalWeight()) {
            return totalWeight < knapsack.getTotalWeight() ? -1 : 1;
        }
        else return 0;
    }

    public void setTotalWeight(long totalWeight) {
        this.totalWeight = totalWeight;
    }
}

class Thing {

    int weight;
    int cost;
    int number;

    public Thing(int weight, int cost, int number) {
        this.weight = weight;
        this.cost = cost;
        this.number = number;
    }

    public int getWeight() {
        return weight;
    }

    public int getCost() {
        return cost;
    }

    public int getNumber() {
        return number;
    }
}
