pragma solidity ^0.5.0;
pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		assert(x > 0);
		assert(x > 100);
		assert(x >= 0);
	}
}
// ----
// Warning: (106-119): Assertion violation happens here
// Warning: (123-138): Assertion violation happens here
