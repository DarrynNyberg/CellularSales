<?php

/**
 * @file
 * Utility and support functions.
 */


// Removed in drupal 7.
function file_directory_path() {
  return file_stream_wrapper_get_instance_by_uri('public://')->realpath();
}
