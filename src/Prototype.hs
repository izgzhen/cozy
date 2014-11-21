#!/usr/bin/env runhaskell

-- Meaningful names for different things
type Type = String
type Field = String
type QueryVar = String

-- Comparison operators
data Comparison = Eq | Gt
    deriving (Eq, Show)

-- Query language
-- A query is sort of like a predicate that operates over
-- one record with Fields and one set of QueryVars.
data Query =
    T | -- match everything
    F | -- match nothing
    Cmp Field Comparison QueryVar |
    And Query Query
    -- someday maybe we'll support "Or" as well
    deriving (Eq, Show)

-- Execution planning language
-- A plan is a program that returns a collection of elements. "What kind of
-- collection" is determined by the plan itself.
data Plan =
    All |
    None |
    HashLookup Field QueryVar | -- get a singleton collection matching the predicate
    BinarySearch Field Comparison QueryVar | -- get a sorted list of elements
    Filter Plan Query | -- execute the plan, then filter the result according to the predicate
    SubPlan Plan Plan | -- super-tricky: execute the left plan, then execute the right plan on the result
    Intersect Plan Plan -- execute the left plan and right plan, keeping only elements from both result sets
    deriving (Show)

-- Data structures
data DataStructure =
    Ty | -- the type over which the query computes, e.g. "Record"
    Empty | -- an empty collection
    HashMap Field DataStructure |
    SortedList Field DataStructure |
    UnsortedList DataStructure |
    Pair DataStructure DataStructure
    deriving (Show)

-- Cost model
data Cost =
    N | -- total number of elements in the structure
    Factor Rational |
    Log Cost |
    Times Cost Cost |
    Plus Cost Cost
    deriving (Show)

-- For a given plan, extract the data structure necessary to execute it.
structureFor :: Plan -> DataStructure
structureFor p = helper p Ty
    where
        -- There is an asymmetry in the DataStructure type. "Ty"
        -- represents "one record" while the others represent
        -- "a collection of records". This function converts any
        -- DataStructure to a collection.
        mkPoly Ty = UnsortedList Ty
        mkPoly x = x

        helper All                    base = mkPoly base
        helper None                   base = Empty
        helper (HashLookup f v)       base = HashMap f (mkPoly base)
        helper (BinarySearch f cmp v) base = SortedList f base
        helper (Filter p _)           base = helper p base
        helper (SubPlan p1 p2)        base = helper p1 (helper p2 base)
        helper (Intersect p1 p2)      base = Pair (helper p1 base) (helper p2 base)

-- Determine whether a plan does, indeed, answer a query.
-- NOTE: This isn't totally correct; it may return False when
--       the answer is actually True.
answersQuery :: Plan -> Query -> Bool
answersQuery p q = postCond p `implies` q
    where
        implies :: Query -> Query -> Bool
        implies a (And q1 q2) = implies a q1 && implies a q2
        implies a q = any (== q) (simpl a)

        simpl :: Query -> [Query]
        simpl (And q1 q2) = simpl q1 ++ simpl q2
        simpl x = [x]

        postCond :: Plan -> Query
        postCond All = T
        postCond None = F
        postCond (HashLookup f v) = Cmp f Eq v
        postCond (BinarySearch f cmp v) = Cmp f cmp v
        postCond (Filter p q) = And (postCond p) q
        postCond (SubPlan p1 p2) = And (postCond p1) (postCond p2)
        postCond (Intersect p1 p2) = And (postCond p1) (postCond p2)

-- Estimate how long a plan takes to execute.
-- Our overall goal is to find a plan that minimizes this for large N.
cost :: Plan -> Cost
cost p = helper p N
    where
        helper All base = base
        helper None base = Factor 0
        helper (HashLookup _ _) base = Factor 1
        helper (BinarySearch _ _ _) base = Log base
        helper (Filter p _) base = helper p base
        -- The 0.5 in the next line needs some justification...
        -- Basically, in the absence of any other info, we assume that the
        -- probability of an object matching a given predicate is 50%, so
        -- the subplan will execute on (we expect) half the data.
        helper (SubPlan p1 p2) base = Plus (helper p1 base) (helper p2 (Times base (Factor 0.5)))
        helper (Intersect p1 p2) base = Plus (helper p1 base) (helper p2 base)

-- A query. Sort of like "SELECT * WHERE age > x AND name = y" where x and y can vary.
query = And (Cmp "age" Gt "x") (Cmp "name" Eq "y")

-- Various candidate plans to implement said query.
plan1 = Intersect (BinarySearch "age" Gt "x") (HashLookup "name" "y")
plan2 = SubPlan (HashLookup "name" "y") (BinarySearch "age" Gt "x")
plan3 = Filter All query
plan4 = Filter All (Cmp "name" Eq "y")

main :: IO ()
main = do
    putStrLn $ "Query: " ++ show query
    mapM_ printPlanInfo [plan1, plan2, plan3, plan4]
    where
        printPlanInfo p = do
            print p
            putStrLn $ "    -->   structure = " ++ show (structureFor p)
            putStrLn $ "    -->   valid? " ++ show (p `answersQuery` query)
            putStrLn $ "    -->   cost = " ++ show (cost p)