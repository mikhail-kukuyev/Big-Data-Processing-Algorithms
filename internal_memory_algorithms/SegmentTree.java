import java.util.Scanner;

public class SegmentTree {

    private Node[] tree;

    private final int BEGIN = 0;
    private final int END;
    private final int ROOT = 0;

    public static void main(String... arg) throws InterruptedException {
        Thread t = new Thread(null, null, "", 4 * 1024 * 1024) {
            @Override
            public void run() {
                Scanner scanner = new Scanner(System.in);
                int n = scanner.nextInt();
                int q = scanner.nextInt();

                int[] array = new int[n];
                for (int i = 0; i < n; i++) {
                    array[i] = scanner.nextInt();
                }

                SegmentTree tree = new SegmentTree(array.length);
                tree.init(array);

                for (int i = 0; i < q; i++) {
                    int queryType = scanner.nextInt();
                    int num1 = scanner.nextInt();
                    int num2 = scanner.nextInt();

                    switch (queryType) {
                        case 1:
                            tree.update(num1 - 1, num2);
                            break;
                        case 2:
                            tree.increment(num1 - 1, num2 - 1);
                            break;
                        case 3:
//                            System.out.println(String.format("%d - %d: %d", num1 - 1, num2 - 1, tree.sum(num1 - 1, num2 - 1, true)));
                            System.out.println(tree.sum(num1 - 1, num2 - 1, true));
                            break;
                        case 4:
//                            System.out.println(String.format("%d - %d: %d", num1 - 1, num2 - 1, tree.sum(num1 - 1, num2 - 1, false)));
                            System.out.println(tree.sum(num1 - 1, num2 - 1, false));
                            break;
                    }
                }
                scanner.close();
            }
        };
        t.start();
        t.join();
    }

    public SegmentTree(int size) {
        int h = (int) (Math.ceil(Math.log(size) / Math.log(2)));
        int N = 2 * (int) Math.pow(2, h) - 1;
        tree = new Node[N];
        END = size - 1;
    }

    public void init(int[] arr) {
        init(arr, BEGIN, END, ROOT);
    }

    public long sum(int l, int r, boolean forEven) {
        return sum(BEGIN, END, l, r, ROOT, forEven);
    }

    public void increment(int l, int r) {
        increment(BEGIN, END, l, r, ROOT);
    }

    public void update(int pos, int value) {
        update(BEGIN, END, pos, value, ROOT);
    }

    private Node init(int[] arr, int start, int end, int current) {
        if (start == end) {
            tree[current] = new Node(arr[start]);
            return tree[current];
        }

        int middle = middle(start, end);

        tree[current] = new Node(
                init(arr, start, middle, left(current)),
                init(arr, middle + 1, end, right(current)));

        return tree[current];
    }

    private void update(int start, int end, int updatePos, int value, int current) {
        if (updatePos < start || updatePos > end) {
            return;
        }

        push(current);

        if (start == end) {
            tree[current] = new Node(value);
        } else {
            int middle = middle(start, end);
            update(start, middle, updatePos, value, left(current));
            update(middle + 1, end, updatePos, value, right(current));

            refresh(current);
        }
    }

    private long sum(int start, int end, int l, int r, int current, boolean forEven) {
        if (end < l || start > r) {
            return 0;
        }

        push(current);

        if (l <= start && r >= end) {
            refresh(current);
            return forEven ? tree[current].getEvenSum() : tree[current].getOddSum();
        } else {
            int middle = middle(start, end);
            long sum = sum(start, middle, l, r, left(current), forEven) +
            sum(middle + 1, end, l, r, right(current), forEven);
            refresh(current);
            return sum;
        }
    }

    private void increment(int start, int end, int l, int r, int current) {
        if (end < l || start > r) {
            return;
        }

        if (l <= start && r >= end) {
            tree[current].increment();
            push(current);
            refresh(current);
        } else {
            push(current);
            int middle = middle(start, end);
            increment(start, middle, l, r, left(current));
            increment(middle + 1, end, l, r, right(current));

            refresh(current);
        }
    }

    private void refresh(int current) {
        if (left(current) < tree.length && tree[left(current)] != null) {
            tree[current].refresh(tree[left(current)], tree[right(current)]);
        }
    }

    private void push(int current) {
        int increments = tree[current].getIncrements();
        if (increments != 0) {
            if (left(current) < tree.length && tree[left(current)] != null) {
                tree[left(current)].addToIncrements(increments);
                tree[right(current)].addToIncrements(increments);
            } else {
                if (increments % 2 == 1) {
                    swapEvenAndOdd(tree[current]);
                }
                tree[current].setEvenSum(tree[current].getEvenSum() + tree[current].getEvenAmount() * increments);
                tree[current].setOddSum(tree[current].getOddSum() + tree[current].getOddAmount() * increments);
            }
            tree[current].setIncrements(0);
        }
    }

    private void swapEvenAndOdd(Node node) {
        long temp = node.getEvenSum();
        node.setEvenSum(node.getOddSum());
        node.setOddSum(temp);

        int temp2 = node.getEvenAmount();
        node.setEvenAmount(node.getOddAmount());
        node.setOddAmount(temp2);
    }

    private int left(int nodeIndex) {
        return 2 * nodeIndex + 1;
    }

    private int right(int nodeIndex) {
        return 2 * nodeIndex + 2;
    }


    private int middle(int start, int end) {
        return (start + (end - start) / 2);
    }

    class Node {

        private int increments = 0;
        private long oddSum = 0;
        private long evenSum = 0;
        private int oddAmount = 0;
        private int evenAmount = 0;

        public Node() {
        }

        public Node(int value) {
            if (value % 2 == 0) {
                evenSum = value;
                evenAmount = 1;
            } else {
                oddSum = value;
                oddAmount = 1;
            }
        }

        public Node(Node left, Node right) {
            refresh(left, right);
        }

        public void refresh(Node left, Node right) {
            evenSum = left.calculateEvenSum() + right.calculateEvenSum();
            oddSum = left.calculateOddSum() + right.calculateOddSum();
            evenAmount = left.calculateEvenAmount() + right.calculateEvenAmount();
            oddAmount = left.calculateOddAmount() + right.calculateOddAmount();
        }

        public long calculateEvenSum() {
            return increments % 2 == 0
            ? evenSum + evenAmount * increments
            : oddSum + oddAmount * increments;
        }

        public long calculateOddSum() {
            return increments % 2 == 0
            ? oddSum + oddAmount * increments
            : evenSum + evenAmount * increments;
        }

        public int calculateEvenAmount() {
            return increments % 2 == 0 ? evenAmount : oddAmount;
        }

        public int calculateOddAmount() {
            return increments % 2 == 0 ? oddAmount : evenAmount;
        }

        public void addToIncrements(long value) {
            increments += value;
        }

        public void increment() {
            increments++;
        }

        public int getIncrements() {
            return increments;
        }

        public void setIncrements(int increments) {
            this.increments = increments;
        }

        public long getOddSum() {
            return oddSum;
        }

        public void setOddSum(long oddSum) {
            this.oddSum = oddSum;
        }

        public long getEvenSum() {
            return evenSum;
        }

        public void setEvenSum(long evenSum) {
            this.evenSum = evenSum;
        }

        public int getOddAmount() {
            return oddAmount;
        }

        public void setOddAmount(int oddAmount) {
            this.oddAmount = oddAmount;
        }

        public int getEvenAmount() {
            return evenAmount;
        }

        public void setEvenAmount(int evenAmount) {
            this.evenAmount = evenAmount;
        }
    }
}
