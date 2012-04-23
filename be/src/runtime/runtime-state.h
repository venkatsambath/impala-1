// Copyright (c) 2011 Cloudera, Inc. All rights reserved.

#ifndef IMPALA_RUNTIME_RUNTIME_STATE_H
#define IMPALA_RUNTIME_RUNTIME_STATE_H

// needed for scoped_ptr to work on ObjectPool
#include "common/object-pool.h"

#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
// stringstream is a typedef, so can't forward declare it.
#include <sstream>

#include "runtime/exec-env.h"
#include "gen-cpp/Types_types.h"  // for TUniqueId
#include "util/runtime-profile.h"

namespace impala {

class DescriptorTbl;
class ObjectPool;
class Status;
class ExecEnv;
class LlvmCodeGen;

// A collection of items that are part of the global state of a
// query and potentially shared across execution nodes.
class RuntimeState {
 public:
  RuntimeState(const TUniqueId& query_id, bool abort_on_error, int max_errors,
               int batch_size, bool llvm_enabled, ExecEnv* exec_env);

  // RuntimeState for executing queries w/o a from clause.
  RuntimeState();

  // Set per-query state.
  Status Init(const TUniqueId& query_id, bool abort_on_error, int max_errors,
              bool llvm_enabled, ExecEnv* exec_env);

  ObjectPool* obj_pool() const { return obj_pool_.get(); }
  const DescriptorTbl& desc_tbl() const { return *desc_tbl_; }
  void set_desc_tbl(DescriptorTbl* desc_tbl) { desc_tbl_ = desc_tbl; }
  int batch_size() const { return batch_size_; }
  int file_buffer_size() const { return file_buffer_size_; }
  bool abort_on_error() const { return abort_on_error_; }
  int max_errors() const { return max_errors_; }
  const std::vector<std::string>& error_log() const { return error_log_; }
  std::stringstream& error_stream() { return error_stream_; }
  const std::vector<std::pair<std::string, int> >& file_errors() const {
    return file_errors_;
  }
  const TUniqueId& query_id() const { return query_id_; }
  ExecEnv* exec_env() { return exec_env_; }
  DataStreamMgr* stream_mgr() { return exec_env_->stream_mgr(); }
  HdfsFsCache* fs_cache() { return exec_env_->fs_cache(); }
  HBaseTableCache* htable_cache() { return exec_env_->htable_cache(); }
  std::vector<std::string>& created_hdfs_files() { return created_hdfs_files_; }
  std::vector<int64_t>& num_appended_rows() { return num_appended_rows_; }

  // Returns runtime state profile
  RuntimeProfile* runtime_profile() { return &profile_; }

  // Returns CodeGen object.  Returns NULL if codegen is disabled.
  LlvmCodeGen* llvm_codegen() { return codegen_.get(); }

  // Append contents of error_stream_ as one message to error_log_. Clears error_stream_.
  void LogErrorStream();

  // Returns true if the error log has not reached max_errors_.
  bool LogHasSpace() const { return error_log_.size() < max_errors_; }

  // Clears the error log.
  void ClearErrorLog() { error_log_.clear(); }

  // Report that num_errors occurred while parsing file_name.
  void ReportFileErrors(const std::string& file_name, int num_errors);

  // Clear the file errors.
  void ClearFileErrors() { file_errors_.clear(); }

  // Returns the error log lines as a string joined with '\n'.
  std::string ErrorLog() const;

  // Returns a string representation of the file_errors_.
  std::string FileErrors() const;

 private:
  static const int DEFAULT_BATCH_SIZE = 1024;
  static const int DEFAULT_FILE_BUFFER_SIZE = 1024 * 1024;

  DescriptorTbl* desc_tbl_;
  boost::scoped_ptr<ObjectPool> obj_pool_;
  int batch_size_;
  int file_buffer_size_;
  // A buffer for error messages.
  // The entire error stream is written to the error_log_ in log_error_stream().
  std::stringstream error_stream_;
  // Logs error messages.
  std::vector<std::string> error_log_;
  // Stores the number of parse errors per file.
  std::vector<std::pair<std::string, int> > file_errors_;
  // Whether to abort if an error is encountered.
  bool abort_on_error_;
  // Maximum number of errors to log.
  int max_errors_;
  TUniqueId query_id_;
  ExecEnv* exec_env_;
  boost::scoped_ptr<LlvmCodeGen> codegen_;
  // Lists the Hdfs files created by this query (e.g., for inserts).
  std::vector<std::string> created_hdfs_files_;
  // Records the total number of appended rows per created Hdfs file.
  std::vector<int64_t> num_appended_rows_;
  
  RuntimeProfile profile_;

  // prohibit copies
  RuntimeState(const RuntimeState&);

  // set codegen_
  Status CreateCodegen();
};

}

#endif
