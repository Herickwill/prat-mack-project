output "lambda_function_name" {
  value = aws_lambda_function.agua_lambda.function_name
}

output "s3_bucket_url" {
  value = aws_s3_bucket.frontend_bucket.website_endpoint
}

output "dynamodb_table_name" {
  value = aws_dynamodb_table.agua_table.name
}

output "sns_topic_arn" {
  value = aws_sns_topic.alerta_agua.arn
}
