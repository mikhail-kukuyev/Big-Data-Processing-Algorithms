import java.util.Scanner;

public class Cache {

    private int cacheSize = 32768;
    private int banksCount = 4;
    private int lineSize = 64;

    private int linesInBank;

    private Cell[][] cache;

    private long ageCounter = 0;
    private long hitCounter = 0;
    private long missCounter = 0;

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        int cacheSize = scanner.nextInt();
        int banksCount = scanner.nextInt();
        int lineSize = scanner.nextInt();
        int n = scanner.nextInt();

        Cache cache = new Cache(cacheSize, banksCount, lineSize);

        for (int i = 0; i < n; i++) {
            cache.get(scanner.nextInt());
        }

        System.out.println(cache.getHitCounter() + "\t" + cache.getMissCounter());
    }

    public Cache(int cacheSize, int banksCount, int lineSize) {
        this.cacheSize = cacheSize;
        this.banksCount = banksCount;
        this.lineSize = lineSize;

        linesInBank = (cacheSize / lineSize) / banksCount;

        cache = new Cell[banksCount][linesInBank];

        for (int i = 0; i < cache.length; i++) {
            for (int j = 0; j < cache[i].length; j++) {
                cache[i][j] = new Cell(0, -1);
            }
        }
    }

    public boolean get(int address) {
        int lineNumber = address / lineSize;
        int lineIndex = lineNumber % linesInBank;

        int oldestCellIndex = -1;
        long minAge = Long.MAX_VALUE;

        for (int i = 0; i < banksCount; i++) {
            if (cache[i][lineIndex].getLine() == lineNumber) {
                cache[i][lineIndex] = new Cell(++ageCounter, lineNumber);
                hitCounter++;
                return true;
            }
            else if (cache[i][lineIndex].getAge() < minAge) {
                oldestCellIndex = i;
                minAge = cache[i][lineIndex].getAge();
            }
        }
        cache[oldestCellIndex][lineIndex] = new Cell(++ageCounter, lineNumber);
        missCounter++;
        return false;
    }

    public long getHitCounter() {
        return hitCounter;
    }

    public long getMissCounter() {
        return missCounter;
    }

    private class Cell {

        private long age;

        private int line;

        public Cell(long age, int line) {
            this.age = age;
            this.line = line;
        }

        public long getAge() {
            return age;
        }

        public int getLine() {
            return line;
        }
    }
}
