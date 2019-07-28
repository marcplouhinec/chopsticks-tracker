#
# Variables
#
# Author: Marc Plouhinec
#

# To find an instance type that suits your need, log in to the Alibaba Cloud console, browse
# to https://ecs-buy.aliyun.com/wizard/#/postpay and select the "Heterogeneous Computing" architecture.
# Please note that the price varies greatly between regions!
variable "instance_type" {
  description = "ECS instance type with Nvidia GPU."
  default = "ecs.gn5-c4g1.xlarge"
}

# Note: we could also use a key pair to connect to the ECS instance via SSH.
variable "ecs_root_password" {
  description = "Root password of the ECS instance."
  default = "YourR00tP@ssword"
}