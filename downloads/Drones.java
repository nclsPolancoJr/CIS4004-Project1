// Nicolas Polanco
// COP3503
// This program utilizes BFS to go through and return the least amount of remote control presses necessary (shortest path)
// to have all the drones successfully deliver their food

import java.util.*;

public class Drones {

    final public static int[] DX = {0, 0, 1, -1};
    final public static int[] DY = {1, -1, 0, 0};

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        int n = scanner.nextInt();
        scanner.nextLine(); // Consume the newline character

        char[][] grid = new char[8][8];
        int[] groupLocations = new int[n];

        for (int i = 0; i < 8; i++) {
            String line = scanner.nextLine();
            for (int j = 0; j < 8; j++) {
                grid[i][j] = line.charAt(j);

                for (int k = 0; k < n; k++) {
                    if (grid[i][j] == ('D' + (char) ('0' + k))) {
                        groupLocations[k] = (i << 3) + j;
                    }
                }
            }
        }

        int result = solve(grid, groupLocations, n);
        System.out.println(result);
    }

    private static int solve(char[][] grid, int[] groupLocations, int n) {
        int[] dx = {0, 0, 1, -1};
        int[] dy = {1, -1, 0, 0};

        // Calculate the starting state based on the positions of drones
        int start = 0;
        for (int i = 0; i < n; i++) {
            // Shift the row and column numbers of each drone to their respective positions in the state integer
            start |= groupLocations[i] << (i * 6); // 6 bits are used to represent the position of each drone
        }

        // Initialize distance array to store the minimum number of remote control button presses
        int[] distance = new int[1 << (6 * n)]; // Each drone position requires 6 bits, so we need 6n bits for n drones
        Queue<Integer> queue = new LinkedList<>();
        queue.offer(start);
          distance[start] = 0;

        while (!queue.isEmpty()) {
            int current = queue.poll();

            int[] positions = new int[n];
            for (int i = 0; i < n; i++) {
                positions[i] = (current >> (i * 6)) & 0x3F; // Extracting 6 bits starting from bit position i*6
            }

            for (int i = 0; i < n; i++) {
                for (int j = 0; j < 4; j++) {
                    int newX = positions[i] / 8 + dx[j]; // Calculate new row number
                    int newY = positions[i] % 8 + dy[j]; // Calculate new column number

                    // Check if the new position is within the grid boundaries and not a no-fly zone
                    if (newX >= 0 && newX < 8 && newY >= 0 && newY < 8 && grid[newX][newY] != 'X') {
                        int newState = current & ~(0x3F << (i * 6)); // Clear the bits representing the old position
                        positions[i] = newX * 8 + newY; // Update the position of the drone
                        newState |= positions[i] << (i * 6); // Set the bits representing the new position

                        // If the new state has not been visited yet, update the distance and add it to the queue
                        if (distance[newState] == 0) {
                            distance[newState] = distance[current] + 1;
                            queue.offer(newState);
                        }
                    }
                }
            }
        }

        // Calculate the final state representing all drones reaching their destinations
        int finalState = 0;
        for (int i = 0; i < n; i++) {
            // Set the final position of each drone in the final state
            finalState |= ((7 << 3) + (groupLocations[i] % 8)) << (i * 6); // Set the bits representing the final position
        }

        // Return the minimum number of button presses required to reach the final state
        return distance[finalState] == 0 ? -1 : distance[finalState];
    }
}
