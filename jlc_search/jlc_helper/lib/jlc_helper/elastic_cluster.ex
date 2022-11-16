defmodule JlcHelper.ElasticsearchCluster do
  use Elasticsearch.Cluster, otp_app: :jlc_helper
end
