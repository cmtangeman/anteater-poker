#define MAX_HAND_RANGE 1770

typedef struct {
    Card c1, c2;
    float weight;
} WeightedHand;

typedef struct {
    WeightedHand hands[MAX_HAND_RANGE];
    int count;
} Range;