/*
 * Copyright (C) 2025 Валери Владев <vdev@lavabit.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * Minimal path compression algorithm for network routes.
 * Core idea: Use bitmasks (letters_table) and bit counting (count_uper_bits)
 * to compress shared prefixes in sequences (e.g., routes [0,1,2]).
 *
 * Use case: Efficient storage of network paths in routing tables.
 * Extensions:
 * - Add search_path()
 * - Add deletion or dynamic updates.
 * - Optimize for large networks (LLA > 64) with dynamic bitmasks.
 * - Integrate with Linux networking (e.g., iproute2).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define LLA 64 // Maximum elements (limited by uint64_t)

struct node {
    uint64_t letters_table; // Bitmask for elements
    struct node* next;      // Next node
    uint32_t weight;        // Weight (e.g., latency)
};

/**
 * Counts bits set from pos+1 to LLA-1 for prefix compression.
 */
static int count_uper_bits(struct node* i, int pos) {
    int count = 0;
    for (int j = pos + 1; j < LLA; j++) {
        if (i->letters_table & (1ULL << j)) {
            count++;
        }
    }
    return count;
}

/**
 * Inserts a path with prefix compression.
 * @param root Pointer to root node.
 * @param path Array of element IDs (e.g., [0,1,2]).
 * @param path_len Length of path.
 * @param weight Weight (e.g., latency).
 */
void insert_path(struct node* root, int* path, int path_len, int weight) {
    struct node* i = root;
    for (int k = 0; k < path_len; k++) {
        int pos = path[k];
        i->letters_table |= (1ULL << pos);
        int n = count_uper_bits(i, pos);
        if (n > 0) {
            for (int j = n; j > 0; j--) {
                i = i->next;
            }
        } else {
            struct node* new_node = calloc(1, sizeof(struct node));
            if (!new_node) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            struct node* temp = i->next;
            i->next = new_node;
            i = i->next;
            i->next = temp;
        }
    }
    i->letters_table |= (1ULL << LLA);
    i->weight = weight;
}

/**
 * Frees the path structure.
 * @param root Pointer to root node.
 */
void free_path(struct node* root) {
    struct node* current = root;
    while (current) {
        struct node* temp = current;
        current = current->next;
        free(temp);
    }
}

/**
 * Tests for insert_path functionality.
 * Community can extend with search_path() or additional tests.
 */
int main() {
    struct node* root = calloc(1, sizeof(struct node));
    assert(root != NULL && "Failed to allocate root node");

    // Test 1: Insert single path
    int path1[] = {0, 1};
    insert_path(root, path1, 2, 10);
    assert(root->letters_table & (1ULL << 0) && "Path [0,1] failed to set bit 0");
    assert(root->next && "Path [0,1] failed to create next node");
    assert(root->next->letters_table & (1ULL << 1) && "Path [0,1] failed to set bit 1");
    assert(root->next->letters_table & (1ULL << LLA) && "Path [0,1] failed to mark end");
    assert(root->next->weight == 10 && "Path [0,1] failed to set weight");

    // Test 2: Insert overlapping path
    int path2[] = {0, 1, 2};
    insert_path(root, path2, 3, 25);
    assert(root->next->next && "Path [0,1,2] failed to create second next node");
    assert(root->next->next->letters_table & (1ULL << 2) && "Path [0,1,2] failed to set bit 2");
    assert(root->next->next->letters_table & (1ULL << LLA) && "Path [0,1,2] failed to mark end");
    assert(root->next->next->weight == 25 && "Path [0,1,2] failed to set weight");

    // Test 3: Insert non-overlapping path
    int path3[] = {1, 2};
    insert_path(root, path3, 2, 15);
    assert(root->letters_table & (1ULL << 1) && "Path [1,2] failed to set bit 1");

    // Clean up
    free_path(root);
    printf("All tests passed\n");

    return 0;
}
