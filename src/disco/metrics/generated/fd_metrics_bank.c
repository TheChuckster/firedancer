/* THIS FILE IS GENERATED BY gen_metrics.py. DO NOT HAND EDIT. */
#include "fd_metrics_bank.h"

const fd_metrics_meta_t FD_METRICS_BANK[FD_METRICS_BANK_TOTAL] = {
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_SANITIZE_FAILURE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_NOT_EXECUTED_FAILURE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, SLOT_ACQUIRE_SUCCESS ),
    DECLARE_METRIC_COUNTER( BANK_TILE, SLOT_ACQUIRE_TOO_HIGH ),
    DECLARE_METRIC_COUNTER( BANK_TILE, SLOT_ACQUIRE_TOO_LOW ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ADDRESS_TABLES_SUCCESS ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ADDRESS_TABLES_SLOT_HASHES_SYSVAR_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ADDRESS_TABLES_ACCOUNT_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ADDRESS_TABLES_INVALID_ACCOUNT_OWNER ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ADDRESS_TABLES_INVALID_ACCOUNT_DATA ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ADDRESS_TABLES_INVALID_INDEX ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_SUCCESS ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ACCOUNT_IN_USE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ACCOUNT_LOADED_TWICE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ACCOUNT_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_PROGRAM_ACCOUNT_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INSUFFICIENT_FUNDS_FOR_FEE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INVALID_ACCOUNT_FOR_FEE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ALREADY_PROCESSED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_BLOCKHASH_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INSTRUCTION_ERROR ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_CALL_CHAIN_TOO_DEEP ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_MISSING_SIGNATURE_FOR_FEE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INVALID_ACCOUNT_INDEX ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_SIGNATURE_FAILURE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INVALID_PROGRAM_FOR_EXECUTION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_SANITIZE_FAILURE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_CLUSTER_MAINTENANCE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ACCOUNT_BORROW_OUTSTANDING ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_WOULD_EXCEED_MAX_BLOCK_COST_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_UNSUPPORTED_VERSION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INVALID_WRITABLE_ACCOUNT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_WOULD_EXCEED_MAX_ACCOUNT_COST_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_WOULD_EXCEED_ACCOUNT_DATA_BLOCK_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_TOO_MANY_ACCOUNT_LOCKS ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_ADDRESS_LOOKUP_TABLE_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INVALID_ADDRESS_LOOKUP_TABLE_OWNER ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INVALID_ADDRESS_LOOKUP_TABLE_DATA ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INVALID_ADDRESS_LOOKUP_TABLE_INDEX ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INVALID_RENT_PAYING_ACCOUNT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_WOULD_EXCEED_MAX_VOTE_COST_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_WOULD_EXCEED_ACCOUNT_DATA_TOTAL_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_DUPLICATE_INSTRUCTION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INSUFFICIENT_FUNDS_FOR_RENT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_MAX_LOADED_ACCOUNTS_DATA_SIZE_EXCEEDED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_INVALID_LOADED_ACCOUNTS_DATA_SIZE_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_RESANITIZATION_NEEDED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_PROGRAM_EXECUTION_TEMPORARILY_RESTRICTED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_LOAD_UNBALANCED_TRANSACTION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_SUCCESS ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_ACCOUNT_IN_USE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_ACCOUNT_LOADED_TWICE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_ACCOUNT_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_PROGRAM_ACCOUNT_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INSUFFICIENT_FUNDS_FOR_FEE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INVALID_ACCOUNT_FOR_FEE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_ALREADY_PROCESSED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_BLOCKHASH_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INSTRUCTION_ERROR ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_CALL_CHAIN_TOO_DEEP ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_MISSING_SIGNATURE_FOR_FEE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INVALID_ACCOUNT_INDEX ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_SIGNATURE_FAILURE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INVALID_PROGRAM_FOR_EXECUTION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_SANITIZE_FAILURE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_CLUSTER_MAINTENANCE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_ACCOUNT_BORROW_OUTSTANDING ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_WOULD_EXCEED_MAX_BLOCK_COST_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_UNSUPPORTED_VERSION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INVALID_WRITABLE_ACCOUNT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_WOULD_EXCEED_MAX_ACCOUNT_COST_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_WOULD_EXCEED_ACCOUNT_DATA_BLOCK_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_TOO_MANY_ACCOUNT_LOCKS ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_ADDRESS_LOOKUP_TABLE_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INVALID_ADDRESS_LOOKUP_TABLE_OWNER ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INVALID_ADDRESS_LOOKUP_TABLE_DATA ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INVALID_ADDRESS_LOOKUP_TABLE_INDEX ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INVALID_RENT_PAYING_ACCOUNT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_WOULD_EXCEED_MAX_VOTE_COST_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_WOULD_EXCEED_ACCOUNT_DATA_TOTAL_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_DUPLICATE_INSTRUCTION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INSUFFICIENT_FUNDS_FOR_RENT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_MAX_LOADED_ACCOUNTS_DATA_SIZE_EXCEEDED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_INVALID_LOADED_ACCOUNTS_DATA_SIZE_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_RESANITIZATION_NEEDED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_PROGRAM_EXECUTION_TEMPORARILY_RESTRICTED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTING_UNBALANCED_TRANSACTION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_SUCCESS ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_ACCOUNT_IN_USE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_ACCOUNT_LOADED_TWICE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_ACCOUNT_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_PROGRAM_ACCOUNT_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INSUFFICIENT_FUNDS_FOR_FEE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INVALID_ACCOUNT_FOR_FEE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_ALREADY_PROCESSED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_BLOCKHASH_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INSTRUCTION_ERROR ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_CALL_CHAIN_TOO_DEEP ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_MISSING_SIGNATURE_FOR_FEE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INVALID_ACCOUNT_INDEX ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_SIGNATURE_FAILURE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INVALID_PROGRAM_FOR_EXECUTION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_SANITIZE_FAILURE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_CLUSTER_MAINTENANCE ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_ACCOUNT_BORROW_OUTSTANDING ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_WOULD_EXCEED_MAX_BLOCK_COST_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_UNSUPPORTED_VERSION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INVALID_WRITABLE_ACCOUNT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_WOULD_EXCEED_MAX_ACCOUNT_COST_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_WOULD_EXCEED_ACCOUNT_DATA_BLOCK_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_TOO_MANY_ACCOUNT_LOCKS ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_ADDRESS_LOOKUP_TABLE_NOT_FOUND ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INVALID_ADDRESS_LOOKUP_TABLE_OWNER ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INVALID_ADDRESS_LOOKUP_TABLE_DATA ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INVALID_ADDRESS_LOOKUP_TABLE_INDEX ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INVALID_RENT_PAYING_ACCOUNT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_WOULD_EXCEED_MAX_VOTE_COST_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_WOULD_EXCEED_ACCOUNT_DATA_TOTAL_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_DUPLICATE_INSTRUCTION ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INSUFFICIENT_FUNDS_FOR_RENT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_MAX_LOADED_ACCOUNTS_DATA_SIZE_EXCEEDED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_INVALID_LOADED_ACCOUNTS_DATA_SIZE_LIMIT ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_RESANITIZATION_NEEDED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_PROGRAM_EXECUTION_TEMPORARILY_RESTRICTED ),
    DECLARE_METRIC_COUNTER( BANK_TILE, TRANSACTION_EXECUTED_UNBALANCED_TRANSACTION ),
    DECLARE_METRIC_HISTOGRAM_SECONDS( BANK_TILE, PROCESSING_TIME ),
};
