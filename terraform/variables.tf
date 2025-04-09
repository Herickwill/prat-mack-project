variable "s3_bucket_name" {
  description = "Nome do bucket S3 para o frontend"
  type        = string
}

variable "dynamodb_table_name" {
  description = "Nome da tabela DynamoDB para armazenar leituras de água"
  type        = string
}

variable "sns_topic_name" {
  description = "Nome do tópico SNS para alertas"
  type        = string
  default     = "agua-alertas"
}

variable "alert_email" {
  description = "E-mail que vai receber os alertas do SNS"
  type        = string
}

variable "lambda_function_name" {
  description = "Nome da função Lambda"
  default     = "monitoramento-agua"
}
